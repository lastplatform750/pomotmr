#include <ncurses.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "logging.h"
#include "cl_args.h"
#include "timer.h"
#include "interface.h"
#include "defaults.h"
#include "server.h"

static bool sig_raised = false;

static void sig_handler(int sig) {
    (void) sig;
    sig_raised = true;
}


int main(int argc, char* argv[]) {
    // Setup signal handling to make sure the socket file gets
    // deleted on exit
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    pomo_timer* tmr = NULL;
    interface*  ui  = NULL;
    server*     ts  = NULL;

    char input      = '\0';

    cl_args* opts = get_cl_args(argc, argv);

    if (opts == NULL) {
        LOG("ERROR: get_cl_args");
        goto error_cleanup;
    }

    tmr = new_timer(opts -> num_short_breaks,
                    opts -> short_break_length,
                    opts -> long_break_length,
                    opts -> focus_length,
                    opts -> alarm_enabled,
                    opts -> alarm_path);
    
    if (tmr == NULL) {
        LOG("ERROR: new_timer");
        goto error_cleanup;
    }

    ui = new_interface();

    if (ui == NULL) {
        LOG("ERROR: new_interface");
        goto error_cleanup;
    }

    if (opts -> server_enabled == true) {
        ts = new_server(opts -> socket_path);
        if (ts == NULL) {
            LOG("ERROR: new_server, disabling server");
            opts -> server_enabled = false;
        } else {
            if (opts -> socket_path != NULL) free(opts -> socket_path);
            opts -> socket_path = NULL;
            if (start_server(ts) == -1) {
                LOG("ERROR: start_server, disabling server");
                opts -> server_enabled = false;
            }
        }
    }

    start_interface(ui, tmr);
    
    while (input != 'q' && sig_raised == false) {
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
        if (opts -> server_enabled == true) {
            update_server(ts, tmr);
        }
        
    }

    if (opts -> server_enabled == true) {
        stop_server(ts);
    }
    
    del_server(ts);
    del_interface(ui);
    del_timer(tmr);
    del_args(opts);

    return 0;

    error_cleanup:
        LOG("Program Crashed!");
        del_server(ts);
        del_interface(ui);
        del_timer(tmr);
        del_args(opts);
        return -1;
}