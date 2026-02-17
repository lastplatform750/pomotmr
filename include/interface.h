#pragma once

#include <ncurses.h>

#include "timer.h"

typedef enum { STATE, TASK } indicator;

typedef struct interface {
  bool started;
  uint indicator_pos[2];
} interface;

interface* new_interface();

int start_interface(interface* ui, pomo_timer* tmr);

int del_interface(interface* ui);

int update_ui(interface* ui, pomo_timer* tmr);