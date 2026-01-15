#include <ncurses.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "logging.h"
#include "cl_args.h"
#include "timer.h"
#include "interface.h"
#include "defaults.h"


int main(int argc, char* argv[]) {
    pomo_timer* tmr = NULL;
    interface* ui   = NULL;
    char input      = '\0';

    cl_args opts;

    get_cl_args(&opts, argc, argv);

    tmr = new_timer(opts.num_short_breaks,
                    opts.short_break_length,
                    opts.long_break_length,
                    opts.focus_length,
                    opts.alarm_filename);
    
    if (tmr == NULL) {
        LOG("ERROR: new_timer");
        goto error_cleanup;
    }

    ui = new_interface();

    if (ui == NULL) {
        LOG("ERROR: new_interface");
        goto error_cleanup;
    }

    start_interface(ui, tmr);

    while (input != 'q') {
        input = getch();

        switch (input) {
            case TOGGLE:
                toggle_timer(tmr);
                break;
            case SKIP:
                if (tmr -> r_state == RING) {
                    stop_ringer(tmr -> alarm);
                }
                clear_timer(tmr);
                advance_p_state(tmr);
                break;
            case CLEAR:
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
        LOG("Program Crashed!");
        del_timer(tmr);
        end_interface(ui);
        return -1;
}