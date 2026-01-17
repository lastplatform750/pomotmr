#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cl_args.h"
#include "defaults.h"
#include "interface.h"
#include "logging.h"
#include "server.h"
#include "timer.h"

static bool sig_raised = false;

static void sig_handler(int sig) {
  (void)sig;
  sig_raised = true;
}

void del_all(server* ts, pomo_timer* tmr, interface* ui, cl_args* opts) {
  del_server(ts);
  del_interface(ui);
  del_timer(tmr);
  del_args(opts);
}

int main(int argc, char* argv[]) {
  // print errors in a file instead of on the screen
  FILE* error_log = freopen(DEFAULT_ERROR_LOG, "w", stderr);

  if (error_log == NULL) {
    LOG_ERRNO("ERROR: freopen");
  } else {
    // flush on each newline
    setvbuf(stderr, NULL, _IOLBF, 0);
  }

  // Setup signal handling to make sure the socket file gets
  // deleted on exit
  struct sigaction sa;
  sa.sa_handler = sig_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  signal(SIGPIPE, SIG_IGN); // don't exit on pipe issues

  pomo_timer* tmr = NULL;
  interface* ui = NULL;
  server* ts = NULL;

  cl_args* opts = get_cl_args(argc, argv);

  if (opts == NULL) {
    LOG("ERROR: get_cl_args");
    goto error_cleanup;
  }

  tmr = new_timer(opts->num_short_breaks, opts->short_break_length,
                  opts->long_break_length, opts->focus_length,
                  opts->alarm_enabled, opts->alarm_path);

  if (tmr == NULL) {
    LOG("ERROR: new_timer");
    goto error_cleanup;
  }

  ui = new_interface();

  if (ui == NULL) {
    LOG("ERROR: new_interface");
    goto error_cleanup;
  }

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

  start_interface(ui, tmr);
  update_ui(ui, tmr);

  char input = '\0';

  while (input != 'q' && sig_raised == false) {
    input = getch();

    switch (input) {
    case TOGGLE:
      toggle_timer(tmr);
      break;
    case SKIP:
      if (tmr->r_state == RING) {
        stop_ringer(tmr->alarm);
      }
      clear_timer(tmr);
      advance_p_state(tmr);
      break;
    case CLEAR:
      if (tmr->r_state != RING) {
        clear_timer(tmr);
      }
      break;
    }

    update_timer(tmr);
    update_ui(ui, tmr);
    if (opts->server_enabled == true) {
      update_server(ts, tmr);
    }
  }

  del_all(ts, tmr, ui, opts);

  // unlink the error log if its empty
  struct stat st;
  if (stat(DEFAULT_ERROR_LOG, &st) == 0 && st.st_size == 0) {
    unlink(DEFAULT_ERROR_LOG);
  }

  return 0;

error_cleanup:
  LOG("Program Crashed!");
  del_all(ts, tmr, ui, opts);
  return -1;
}