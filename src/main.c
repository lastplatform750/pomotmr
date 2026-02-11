#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "cl_args.h"
#include "defaults.h"
#include "input.h"
#include "interface.h"
#include "error_log.h"
#include "server.h"
#include "sig_handling.h"
#include "timer.h"

void del_all(server* ts, pomo_timer* tmr, interface* ui, cl_args* opts,
             error_log* log) {
  del_server(ts);
  del_interface(ui);
  del_timer(tmr);
  del_args(opts);
  close_log(log);
}

int main(int argc, char* argv[]) {
  // exit if not run in a tty or if launched by wmenu
  if (getenv("XDG_ACTIVATION_TOKEN") != NULL || !isatty(STDIN_FILENO) ||
      !isatty(STDOUT_FILENO)) {
    return -1;
  }

  // print errors in a file instead of on the screen
  error_log* log = open_log(argc, argv);

  // Setup signal handling to make sure the socket file gets
  // deleted on exit
  start_sig_handling();

  pomo_timer* tmr = NULL;
  interface* ui = NULL;
  server* ts = NULL;

  // get the command line options
  cl_args* opts = get_cl_args(argc, argv);

  if (opts == NULL) {
    LOG("ERROR: get_cl_args");
    goto error_cleanup;
  }

  // create the timer
  tmr = new_timer(opts->num_short_breaks, opts->short_break_length,
                  opts->long_break_length, opts->focus_length,
                  opts->alarm_enabled, opts->alarm_path);

  if (tmr == NULL) {
    LOG("ERROR: new_timer");
    goto error_cleanup;
  }

  // create the interface
  ui = new_interface();

  if (ui == NULL) {
    LOG("ERROR: new_interface");
    goto error_cleanup;
  }

  // create the server (if enabled)
  if (opts->server_enabled == true) {
    ts = new_server(opts->socket_path);
    if (ts == NULL) {
      LOG("ERROR: new_server, disabling server");
      opts->server_enabled = false;
    } else {
      // don't need socket_path anymore, clear it
      if (opts->socket_path != NULL)
        free(opts->socket_path);
      opts->socket_path = NULL;
    }
  }

  // start the interface
  start_interface(ui, tmr);
  update_ui(ui, tmr);

  // main loop - wait for input until timeout
  // then handle input if something was inputted
  // then update everything
  char input = '\0';
  while (input != QUIT && exit_sig_raised == false) {
    input = getch();

    handle_input(input, tmr);

    update_timer(tmr);
    update_ui(ui, tmr);
    if (opts->server_enabled == true) {
      update_server(ts, tmr);
    }
  }

  del_all(ts, tmr, ui, opts, log);

  return 0;

error_cleanup:
  LOG("Program Crashed!");
  del_all(ts, tmr, ui, opts, log);
  return -1;
}