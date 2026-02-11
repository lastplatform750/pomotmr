#include <bits/time.h>
#include <ncurses.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <time.h>

#include "error_log.h"
#include "ring.h"
#include "timer.h"

const char* const POMO_STATE_STRINGS[] = {"Short Break", "Long Break", "Focus"};

pomo_timer* new_timer(uint num_short_breaks, int short_break_length,
                      int long_break_length, int focus_length,
                      bool alarm_enabled, char* alarm_path) {
  pomo_timer* tmr = (pomo_timer* )malloc(sizeof(pomo_timer));

  if (tmr == NULL) {
    LOG_ERRNO("ERROR: malloc");
    return NULL;
  }

  tmr->r_state = PAUSE;
  tmr->p_state = FOCUS;

  tmr->previous_elapsed_time = 0u;
  tmr->start_time = 0;

  tmr->break_counter = 0u;
  tmr->num_short_breaks = num_short_breaks;
  tmr->break_lengths[SHORT_BREAK] = short_break_length;
  tmr->break_lengths[LONG_BREAK] = long_break_length;
  tmr->break_lengths[FOCUS] = focus_length;

  tmr->alarm_enabled = alarm_enabled;

  if (alarm_enabled && alarm_path != NULL) {
    tmr->alarm = new_ringer(alarm_path);
    if (tmr->alarm == NULL) {
      LOG("ERROR: new_ringer, disabling alarm");
      tmr->alarm_enabled = false;
    }
  } else {
    tmr->alarm = NULL; // alarm disabled
  }

  return tmr;
}

void del_timer(pomo_timer* tmr) {
  if (tmr != NULL) {
    if ((tmr->alarm) != NULL)
      del_ringer(tmr->alarm);
    free(tmr);
  }
}

int get_mono_time() {
  timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0) {
    LOG_ERRNO("ERROR: clock_gettime");
    return -1;
  }
  return (int)tp.tv_sec;
}

int get_elapsed_time(pomo_timer* tmr) {
  if (tmr->r_state == PAUSE || tmr->r_state == RING) {
    return tmr->previous_elapsed_time;
  }

  int mono_time;
  if ((mono_time = get_mono_time()) == -1) {
    LOG("ERROR: get_mono_time");
    return -1;
  }

  return (mono_time - (tmr->start_time)) + (tmr->previous_elapsed_time);
}

int get_remaining_time(pomo_timer* tmr) {
  int elapsed_time;
  if ((elapsed_time = get_elapsed_time(tmr)) == -1) {
    LOG("ERROR: get_elapsed_time");
    return -1;
  }
  return (tmr->break_lengths[tmr->p_state]) - elapsed_time;
}

int stop_timer(pomo_timer* tmr) {
  if (tmr->r_state == PLAY) {
    int elapsed_time = get_elapsed_time(tmr);

    if (elapsed_time < 0) {
      LOG("ERROR: get_elapsed_time");
      return -1;
    }

    tmr->previous_elapsed_time = elapsed_time;
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
  tmr->r_state = PAUSE;
  tmr->previous_elapsed_time = 0;
  tmr->start_time = 0;
}

const char* get_p_state_string(pomo_timer* tmr) {
  return POMO_STATE_STRINGS[(int)tmr->p_state];
}

void advance_p_state(pomo_timer* tmr) {
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
  if (get_elapsed_time(tmr) > (tmr->break_lengths[tmr->p_state])) {
    clear_timer(tmr);
    advance_p_state(tmr);

    // play alarm sound
    tmr->r_state = RING;
    if ((tmr->alarm_enabled) == true) {
      start_ringer(tmr->alarm);
    }
  }
}