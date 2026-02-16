#include "input.h"
#include "defaults.h"
#include "error_log.h"

int handle_input(char input, pomo_timer* tmr) {
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
  }

  return 0;
}