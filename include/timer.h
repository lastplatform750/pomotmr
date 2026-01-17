#pragma once

#include <pthread.h>
#include <stdatomic.h>

#include "ring.h"
#include "typedefs.h"

extern const char* const POMO_STATE_STRINGS[];

typedef enum { PLAY, PAUSE, RING } run_state;

typedef enum { SHORT_BREAK, LONG_BREAK, FOCUS } pomo_state;

typedef struct {
  uint num_short_breaks;
  uint break_counter;

  // These are ints cause of error checking
  // and to avoid comparisons betweens ints and uints
  int previous_elapsed_time;
  int start_time;
  int break_lengths[3];

  run_state r_state;
  pomo_state p_state;

  bool alarm_enabled;
  ringer* alarm;
} pomo_timer;

pomo_timer* new_timer(uint num_short_breaks, int short_break_length,
                      int long_break_length, int focus_length,
                      bool alarm_enabled, char* alarm_path);

// free the memory for tmr and clear the pointer
void del_timer(pomo_timer* tmr);

// returns elapsed time in seconds
int get_elapsed_time(pomo_timer* tmr);

// returns remaining time in pomo section
int get_remaining_time(pomo_timer* tmr);

// stops the timer but retains amount of previous elapsed time
int stop_timer(pomo_timer* tmr);

// starts running the timer from the moment this is called
int start_timer(pomo_timer* tmr);

int toggle_timer(pomo_timer* tmr);

// stops the timer AND clears amount of previous elapsed time
void clear_timer(pomo_timer* tmr);

// prints a string describing the pomo state
const char* get_p_state_string(pomo_timer* tmr);

// moves up to the next pomo_state
// does NOT change the run_state
void advance_p_state(pomo_timer* tmr);

// gets the time and advances p_state if the state's time has elapsed
void update_timer(pomo_timer* tmr);
