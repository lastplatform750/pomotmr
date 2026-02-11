#include <ncurses.h>
#include <stdlib.h>

#include "interface.h"
#include "error_log.h"
#include "sig_handling.h"
#include "timer.h"

// How long getch will block
const uint TIMEOUT_LENGTH = 500;

const uint COL_BLOCK_1 = 2;
const uint COL_BLOCK_2 = 30;

void clear_line(uint row) {
  move(row, 0);
  clrtoeol();
}

int move_indicator(interface* ui, uint new_pos) {
  if (ui->indicator_pos == new_pos) {
    return 0;
  }

  mvaddch(ui->indicator_pos, COL_BLOCK_1, ' ');
  mvaddch(new_pos, COL_BLOCK_1, '*');

  ui->indicator_pos = new_pos;

  return 0;
}

interface* new_interface() {
  interface* ui = (interface* )calloc(1, sizeof(interface));
  if (ui == NULL) {
    LOG_ERRNO("ERROR: calloc");
  }
  return ui;
}

void init_draw(interface* ui, pomo_timer* tmr) {
  uint i;
  for (i = 0; i < (tmr->num_short_breaks); i++) {
    mvprintw(10 + 2 * i, COL_BLOCK_1 + 2, "FOCUS");
    mvprintw(11 + 2 * i, COL_BLOCK_1 + 2, "SHORT_BREAK");
  }
  mvprintw(10 + 2 * i, COL_BLOCK_1 + 2, "FOCUS");
  mvprintw(11 + 2 * i, COL_BLOCK_1 + 2, "LONG_BREAK");

  move_indicator(ui, 10);
}

int start_interface(interface* ui, pomo_timer* tmr) {
  initscr();            // start curses mode
  cbreak();             // disable line buffering
  noecho();             // don't echo typed characters
  keypad(stdscr, TRUE); // enable arrow keys (don't need?)
  // nodelay(stdscr, TRUE); // don't block for input
  curs_set(0); // hide cursor
  timeout(TIMEOUT_LENGTH);

  ui->started = true;

  init_draw(ui, tmr);

  return 0;
}

int del_interface(interface* ui) {
  endwin();
  if (ui != NULL)
    free(ui);
  return 0;
}

int update_ui(interface* ui, pomo_timer* tmr) {
  if (winch_sig_raised == true) {
    clear();
    init_draw(ui, tmr);
    winch_sig_raised = false;
  }

  clear_line(1);
  clear_line(3);
  clear_line(5);

  int remaining_time = get_remaining_time(tmr);

  mvprintw(1, COL_BLOCK_1, "Remaining time: %i:%02d", remaining_time / 60,
           remaining_time % 60);
  mvprintw(3, COL_BLOCK_1, "%s", get_p_state_string(tmr));

  switch (tmr->r_state) {
  case RING:
    mvprintw(5, COL_BLOCK_1, "PAUSED");
    mvprintw(7, COL_BLOCK_1, "RINGING");
    break;
  case PAUSE:
    mvprintw(5, COL_BLOCK_1, "PAUSED");
    clear_line(7);
    break;
  case PLAY:
    mvprintw(5, COL_BLOCK_1, "PLAYING");
    clear_line(7);
  }

  if (tmr->p_state == FOCUS) {
    move_indicator(ui, 10 + 2 * (tmr->break_counter));
  } else {
    move_indicator(ui, 11 + 2 * (tmr->break_counter));
  }

  refresh();
  return 0;
}
