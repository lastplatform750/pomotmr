#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "timer.h"
#include "interface.h"

int main(int argc, char* argv[]) {
    pomo_timer* tmr = NULL;
    interface* ui   = NULL;
    char input      = '\0';

    uint num_short_breaks   = DEFAULT_NUM_SHORT_BREAKS;
    uint short_break_length = DEFAULT_SHORT_BREAK_LENGTH;
    uint long_break_length  = DEFAULT_LONG_BREAK_LENGTH;
    uint focus_length       = DEFAULT_FOCUS_LENGTH;

    if (argc > 1) {
        if (!strcmp(argv[1], "short")) {
            short_break_length = 2;
            long_break_length  = 3;
            focus_length       = 4;
        }
    }

    tmr = new_timer(num_short_breaks,
                        short_break_length,
                        long_break_length,
                        focus_length);
    
    if (tmr == NULL) {
        fprintf(stderr, "ERROR: in new_timer\n");
        goto error_cleanup;
    }

    ui = new_interface();

    if (ui == NULL) {
        fprintf(stderr, "ERROR: in new_interface\n");
        goto error_cleanup;
    }

    start_interface(ui, tmr);
    start_timer(tmr);

    while (input != 'q') {
        input = getch();

        switch (input) {
            case ' ':
                toggle_timer(tmr);
                break;
            case '\n':
                if (tmr -> r_state == RING) {
                    stop_ringer(tmr -> alarm);
                }
                clear_timer(tmr);
                advance_p_state(tmr);
                break;
            case 'c':
                if (tmr -> r_state != RING) {
                    clear_timer(tmr);
                }
                break;
        }

        update_timer(tmr);
        update_ui(ui, tmr);
    }

    del_timer(tmr);
    end_interface(ui);

    return 0;

    error_cleanup:
        del_timer(tmr);
        end_interface(ui);
        return -1;
}