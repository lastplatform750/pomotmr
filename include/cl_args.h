#pragma once

#include <linux/limits.h>

#include "typedefs.h"

typedef struct {
  uint num_short_breaks;
  int short_break_length;
  int long_break_length;
  int focus_length;
  bool server_enabled;
  bool alarm_enabled;
  bool timer_log_enabled;
  char* alarm_path;
  char* socket_path;
  char* timer_log_path;
} cl_args;

cl_args* get_cl_args(int argc, char* argv[]);

void del_args(cl_args* opts);