#include <bits/time.h>
#include <ncurses.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <time.h>

#include "cl_args.h"
#include "error_log.h"
#include "ring.h"
#include "timer.h"
#include "timer_log.h"

const char* const POMO_STATE_STRINGS[] = {"Short Break", "Long Break", "Focus"};

pomo_timer* new_timer(cl_args* opts) {
  pomo_timer* tmr = (pomo_timer* )calloc(1, sizeof(pomo_timer));

  if (tmr == NULL) {
    LOG_ERRNO("ERROR: calloc");
    return NULL;
  }

  tmr->r_state = PAUSE;
  tmr->p_state = FOCUS;

  tmr->previous_elapsed_time = 0;
  tmr->current_elapsed_time = 0;
  tmr->start_time = 0;

  tmr->break_counter = 0u;
  tmr->num_short_breaks = opts->num_short_breaks;
  tmr->break_lengths[SHORT_BREAK] = opts->short_break_length;
  tmr->break_lengths[LONG_BREAK] = opts->long_break_length;
  tmr->break_lengths[FOCUS] = opts->focus_length;

  tmr->alarm_enabled = opts->alarm_enabled;

  if (tmr->alarm_enabled && opts->alarm_path != NULL) {
    tmr->alarm = new_ringer(opts->alarm_path);
    if (tmr->alarm == NULL) {
      LOG("ERROR: new_ringer, disabling alarm");
      tmr->alarm_enabled = false;
    }
  } else {
    tmr->alarm = NULL; // alarm disabled
    tmr->alarm_enabled = false;
  }

  tmr->timer_log_enabled = opts->timer_log_enabled;

  if (tmr->timer_log_enabled && opts->timer_log_path != NULL) {
    tmr->tlog = new_timer_log(opts);
    if (tmr->tlog == NULL) {
      LOG("ERROR: new_timer_log, disabling timer log");
      tmr->timer_log_enabled = false;
    }
  } else {
    tmr->tlog = NULL;
    tmr->timer_log_enabled = false;
  }

  return tmr;
}

int get_mono_time() {
  timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0) {
    LOG_ERRNO("ERROR: clock_gettime");
    return -1;
  }
  return (int)tp.tv_sec;
}

int update_elapsed_time(pomo_timer* tmr) {
  if (tmr->r_state == PAUSE || tmr->r_state == RING) {
    return 0;
  }

  int mono_time;
  if ((mono_time = get_mono_time()) == -1) {
    LOG("ERROR: get_mono_time");
    return -1;
  }

  tmr->current_elapsed_time = (mono_time - (tmr->start_time));
  tmr->total_elapsed_time =
      tmr->previous_elapsed_time + tmr->current_elapsed_time;

  return 0;
}

int get_remaining_time(pomo_timer* tmr) {
  return (tmr->break_lengths[tmr->p_state]) - tmr->total_elapsed_time;
}

int stop_timer(pomo_timer* tmr) {
  if (tmr->r_state == PLAY) {
    tmr->previous_elapsed_time = tmr->total_elapsed_time;
    tmr->current_elapsed_time = 0;
  }
  tmr->r_state = PAUSE;
  return 0;
}

int start_timer(pomo_timer* tmr) {
  if (tmr->r_state == PAUSE || tmr->r_state == RING) {
    int mono_time;
    if ((mono_time = get_mono_time()) == -1) {
      LOG("ERROR: get_mono_time");
      return -1;
    }
    tmr->start_time = mono_time;
  }
  tmr->r_state = PLAY;
  return 0;
}

int toggle_timer(pomo_timer* tmr) {
  switch (tmr->r_state) {
  case PAUSE:
    if (start_timer(tmr) == -1) {
      LOG("ERROR: start_timer");
      return -1;
    }
    break;
  case PLAY:
    if (stop_timer(tmr) == -1) {
      LOG("ERROR: stop_timer");
      return -1;
    }
    break;
  case RING:
    tmr->r_state = PAUSE;
    if (tmr->alarm_enabled == true) {
      stop_ringer(tmr->alarm);
    }
  }

  return 0;
}

void clear_timer(pomo_timer* tmr) {
  if (tmr->timer_log_enabled) {
    update_timer_log(tmr->tlog, tmr->p_state, tmr->total_elapsed_time);
  }

  tmr->r_state = PAUSE;
  tmr->previous_elapsed_time = 0;
  tmr->current_elapsed_time = 0;
  tmr->total_elapsed_time = 0;
  tmr->start_time = 0;
}

const char* get_p_state_string(pomo_timer* tmr) {
  return POMO_STATE_STRINGS[(int)tmr->p_state];
}

void advance_p_state(pomo_timer* tmr) {
  clear_timer(tmr);

  if (tmr->p_state == SHORT_BREAK || tmr->p_state == LONG_BREAK) {
    tmr->p_state = FOCUS;
    tmr->break_counter =
        ((tmr->break_counter) + 1) % (tmr->num_short_breaks + 1);
  } else if (tmr->break_counter == (tmr->num_short_breaks)) {
    tmr->p_state = LONG_BREAK;
  } else {
    tmr->p_state = SHORT_BREAK;
  }
}

void update_timer(pomo_timer* tmr) {
  update_elapsed_time(tmr);

  if (tmr->total_elapsed_time >= (tmr->break_lengths[tmr->p_state])) {
    advance_p_state(tmr);

    // play alarm sound
    tmr->r_state = RING;
    if (tmr->alarm_enabled) {
      start_ringer(tmr->alarm);
    }
  }
}

void del_timer(pomo_timer* tmr) {
  if (tmr->timer_log_enabled) {
    update_elapsed_time(tmr);
    update_timer_log(tmr->tlog, tmr->p_state, tmr->total_elapsed_time);
  }

  if (tmr != NULL) {
    if ((tmr->alarm) != NULL)
      del_ringer(tmr->alarm);
    if ((tmr->tlog) != NULL)
      del_timer_log(tmr->tlog);
    free(tmr);
  }
}