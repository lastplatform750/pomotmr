#include "timer_log.h"
#include "error_log.h"
#include "typedefs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

timer_log* new_timer_log(cl_args* opts) {
  timer_log* tlog;

  tlog = calloc(1, sizeof(timer_log));

  if (tlog == NULL) {
    LOG_ERRNO("ERROR: calloc");
    goto error_cleanup;
  }

  if (time(&(tlog->log_start_time)) == -1) {
    LOG_ERRNO("ERROR: time");
    goto error_cleanup;
  }

  tlog->timer_log_path = opts->timer_log_path;

  return tlog;

error_cleanup:
  if (tlog != NULL) {
    free(tlog);
  }
  return NULL;
}

void update_timer_log(timer_log* tlog, pomo_state p_state, int add_time) {
  if (p_state == FOCUS) {
    tlog->total_focus_time += add_time;
  } else {
    tlog->total_break_time += add_time;
  }
}

void clear_timer_log(timer_log* tlog) {
  tlog->total_focus_time = 0;
  tlog->total_break_time = 0;
  tlog->log_start_time = 0;
}

int flush_timer_log(timer_log* tlog) {
  if (tlog->total_break_time <= 0 && tlog->total_focus_time <= 0) {
    return 0;
  }

  int fd = open(tlog->timer_log_path, O_RDWR | O_APPEND | O_CREAT, 0666);
  if (fd == -1) {
    LOG_ERRNO("ERROR: open");
    return -1;
  }

  time_t log_end_time;
  if (time(&log_end_time) == -1) {
    LOG_ERRNO("ERROR: time");
    return -1;
  }

  if (dprintf(fd, "%ld,%ld,%i,%i\n", tlog->log_start_time, log_end_time,
              tlog->total_break_time, tlog->total_focus_time) == -1) {
    LOG_ERRNO("ERROR: dprintf");
  }

  clear_timer_log(tlog);

  close(fd);
  return 0;
}

void del_timer_log(timer_log* tlog) {
  if (tlog != NULL) {
    flush_timer_log(tlog);
    free(tlog);
  }
}