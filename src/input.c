#include "input.h"
#include "defaults.h"
#include "error_log.h"
#include "timer.h"
#include "timer_log.h"

int handle_input(int input, pomo_timer* tmr) {
  switch (input) {
  case QUIT:
    break;
  case TOGGLE:
    if (toggle_timer(tmr) == -1) {
      LOG("ERROR: toggle_timer");
    }
    break;
  case SKIP:
    if (tmr->r_state == RING) {
      stop_ringer(tmr->alarm);
    }
    advance_p_state(tmr);
    break;
  case CLEAR:
    if (tmr->r_state != RING) {
      clear_timer(tmr);
    }
    break;
  case SWITCH_TASK:
    if (tmr->timer_log_enabled && tmr->r_state != RING) {
      clear_timer(tmr);
      advance_task(tmr->tlog, 1);
    }
    break;
  case SWITCH_TASK_REVERSE:
    if (tmr->timer_log_enabled && tmr->r_state != RING) {
      clear_timer(tmr);
      advance_task(tmr->tlog, -1);
    }
  }

  return 0;
}