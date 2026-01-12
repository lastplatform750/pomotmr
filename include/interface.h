#pragma once

#include <ncurses.h>

#include "timer.h"

typedef struct interface {
    bool started;
    uint indicator_pos;
} interface;

interface* new_interface();

int start_interface(interface* ui, pomo_timer* tmr);

int end_interface(interface* ui);

int update_ui(interface* ui, pomo_timer* tmr);