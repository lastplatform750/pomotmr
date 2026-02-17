#pragma once

#include <time.h>

#include "cl_args.h"
#include "typedefs.h"

typedef struct {
  char* timer_log_path;
  separated_list* task_names;
  time_t log_start_time;
  int total_focus_time;
  int total_break_time;
  uint current_task;
  bool is_active;
} timer_log;

timer_log* new_timer_log(cl_args* opts);

int start_timer_log(timer_log* tlog);

void update_timer_log(timer_log* tlog, pomo_state p_state, int add_time);

int advance_task(timer_log* tlog);

int flush_timer_log(timer_log* tlog);

void del_timer_log(timer_log* tlog);