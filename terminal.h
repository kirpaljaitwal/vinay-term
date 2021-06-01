#include <gtk/gtk.h>

extern int app_window_width;
extern int app_window_height;

#define VIN_TYPE_TERMINAL vin_terminal_get_type()
G_DECLARE_FINAL_TYPE(VinTerminal,vin_terminal,VIN,TERMINAL,GtkTextView)

struct _PTY
{
  int fdm;
  int fds;
};

typedef struct _PTY PTY;

