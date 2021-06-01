#include <pty.h>

static void spawn_local_pty(PTY *pty)
{
  char *env[] = {"LANG=en_IN","TERM=dumb", NULL};
  pid_t p = fork();
  if(p == 0)
  {// child process
    close(pty->fdm);

    setsid();
    ioctl(0,TIOCSCTTY,1);
    dup2(pty->fds, 0); //standard input
    dup2(pty->fds, 1); //standard output
    dup2(pty->fds, 2); // standard error
    close(pty->fds);

    //execle("/bin/bash", NULL, (char *)NULL, env);
    execvpe("/bin/bash", (char *)NULL, (char *)env);
  }

  else if(p > 0)
  { /* master process */
    close(pty->fds);

    /* Setup Terminal Size*/
    struct winsize size;
    size.ws_xpixel = app_window_width;
    size.ws_ypixel = app_window_height;
    ioctl(pty->fdm, TIOCSWINSZ, &size);
  }

  else
  {
    g_print("Error Occured.\n");
  }
}

static void local_pty_setup(VinTerminal *terminal)
{

  char name[20]="";
  if(openpty(&terminal->pty.fdm, &terminal->pty.fds, name, NULL, NULL) < 0)
  {
    g_print("Unable to open psuedo Terminal");
  }

  else
  {
    spawn_local_pty((PTY *) &terminal->pty);
  }
}
