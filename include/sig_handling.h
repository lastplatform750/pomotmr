#pragma once

#include <signal.h>

extern volatile sig_atomic_t exit_sig_raised;
extern volatile sig_atomic_t winch_sig_raised;

int start_sig_handling();