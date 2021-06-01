#include <pty.h>
#include "terminal.h"
#define CHAR_COUNT 2048
struct _VinTerminal
{
  GtkTextView parent;
  GIOChannel *channel;
  GtkTextBuffer *buffer;
  PTY pty;
};

struct _VinTerminalClass
{
  GtkTextViewClass parent_class;
};

G_DEFINE_TYPE(VinTerminal, vin_terminal, GTK_TYPE_TEXT_VIEW)

#include "pty_local.h"
#include "key_press.h"

static gboolean
vin_terminal_button_press_event(GtkWidget *widget, GdkEventButton *button)
{ return TRUE;}

static void
vin_terminal_scroll_screen(VinTerminal *terminal)
{
  GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(terminal));
  GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
  g_signal_emit_by_name(adjustment, "changed",NULL);
}


static gboolean
vin_terminal_key_press(GtkWidget *widget, GdkEventKey *event)
{
  VinTerminal *terminal = VIN_TERMINAL(widget);
  GdkModifierType modifier;
  modifier = gtk_accelerator_get_default_mod_mask();

  int c=0;
  gchar buf[7] = "\0";

  if( event->keyval == GDK_KEY_c && (event->state & modifier)==GDK_CONTROL_MASK)
  {
    buf[0]=3;
    c++;
  }

  else if(event->keyval == GDK_KEY_Return)
  {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(terminal->buffer, &end);
    gtk_text_iter_forward_cursor_position(&end);
    strcpy(buf, "\n");
    c = strlen(buf);
  }



  else
  {
  c = g_unichar_to_utf8(gdk_keyval_to_unicode(event->keyval),buf);
  }

  buf[c]=0;
  write(terminal->pty.fdm, buf, strlen(buf));
  return FALSE;
}


static gboolean
terminal_char_read(GIOChannel *channel, GIOCondition *condition, gpointer userdata)
{
  VinTerminal *terminal = VIN_TERMINAL(userdata);
  vin_terminal_scroll_screen(terminal);

  gsize buff_len = CHAR_COUNT + 1;
  gchar *buff = (gchar*) g_malloc(buff_len);
  buff[0]=0;

  gsize c=0;
  gsize count=0;
  c = read(terminal->pty.fdm, buff, CHAR_COUNT);
  buff[c]='\0';


  GtkTextIter insert, start, end; //insert point, start point, end point
  gtk_text_buffer_get_iter_at_mark(terminal->buffer, &insert, gtk_text_buffer_get_insert(terminal->buffer));
  start = end = insert;

  if(buff[0]=='\b') // if backspace is got
  {
      gtk_text_iter_backward_cursor_positions(&end,1);
      gtk_text_buffer_delete(terminal->buffer, &start, &end);
  }

  else if(buff[0]=='\a') // if bell is got
  {
    g_print("%0X, ", buff[0]);
  }


  else // append text to the terminal window
  {
    gtk_text_buffer_insert(terminal->buffer, &insert, buff, -1);
  }

  // Scrolling

  g_free(buff);
  return TRUE;
}

static void
vin_terminal_class_init(VinTerminalClass *klass)
{
  GtkWidgetClass *w_class = GTK_WIDGET_CLASS(klass);
  w_class->key_press_event = vin_terminal_key_press;
  w_class->button_press_event = vin_terminal_button_press_event;
  w_class->button_release_event = vin_terminal_button_press_event;

}

static void
vin_terminal_init(VinTerminal *self)
{
  VinTerminal *terminal = VIN_TERMINAL(self);
  terminal->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));

  local_pty_setup(terminal);
  terminal->channel = g_io_channel_unix_new(terminal->pty.fdm);
  int rc = g_io_add_watch(terminal->channel, G_IO_IN | G_IO_HUP , (GIOFunc)terminal_char_read, terminal);

}
