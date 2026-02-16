#pragma once

#include <time.h>

#include "cl_args.h"
#include "typedefs.h"

typedef struct {
  char* timer_log_path;
  time_t log_start_time;
  int total_focus_time;
  int total_break_time;
} timer_log;

timer_log* new_timer_log(cl_args* opts);

void update_timer_log(timer_log* tlog, pomo_state p_state, int add_time);

int flush_timer_log(timer_log* tlog);

void del_timer_log(timer_log* tlog);