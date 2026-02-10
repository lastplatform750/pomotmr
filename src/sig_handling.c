#include <signal.h>
#include <stdio.h>

#include "sig_handling.h"

bool exit_sig_raised = false;
bool winch_sig_raised = false;

static void sig_handler(int sig) {
  switch (sig) {
  case SIGWINCH:
    winch_sig_raised = true;
    break;
  default:
    exit_sig_raised = true;
  }
}

int start_sig_handling() {
  struct sigaction sa;
  sa.sa_handler = sig_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGWINCH, &sa, NULL);
  signal(SIGPIPE, SIG_IGN); // don't exit on pipe issues

  return 0;
}