
#include <gtk/gtk.h>
#include "terminal.h"
#define _GNU_SOURCE

int app_window_width=600;
int app_window_height=400;


static void
scroll_win_cb(GtkAdjustment *adjustmenmt, gpointer userdata)
{
  gdouble upper = gtk_adjustment_get_upper(adjustmenmt);
  gdouble pagesize = gtk_adjustment_get_page_size(adjustmenmt);
  gtk_adjustment_set_value(adjustmenmt, upper - pagesize);
}



static void
activate_app(GApplication *app, gpointer userdata)
{
  /* Main Application Window*/
  GtkWidget *window = g_object_new(GTK_TYPE_APPLICATION_WINDOW,
                                  "application",app,
                                  "title","Vinay Terminal",
                                  "default-width",app_window_width,
                                  "default-height",app_window_height,
                                  NULL);
  /*Scrolled Window*/
  GtkWidget *scrwin = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwin),GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrwin));
  g_signal_connect(adjustment, "changed",G_CALLBACK(scroll_win_cb),NULL);



  GtkWidget *terminal = g_object_new(VIN_TYPE_TERMINAL,
                                    "wrap-mode", GTK_WRAP_WORD_CHAR,
                                    NULL);

  GtkStyleContext *context = gtk_widget_get_style_context(terminal);
  gtk_style_context_add_class(context, "terminal");

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider,".terminal{"
                                                              "color: #222222;"
                                                              "font: 10pt 'Noto Sans Devanagari';"
                                                              //"font-size: 10pt;"
                                                              "}", -1, NULL);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);





  gtk_container_add(GTK_CONTAINER(scrwin),terminal);
  gtk_container_add(GTK_CONTAINER(window),scrwin);


  gtk_widget_show_all(window);
}

int main()
{
  GtkApplication *app = gtk_application_new("org.vinay.terminal",G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app,"activate",G_CALLBACK(activate_app),NULL);
  int status = g_application_run(G_APPLICATION(app),0,0);
  g_object_unref(app);
  return status;
}
