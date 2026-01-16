#include <linux/limits.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "logging.h"
#include "cl_args.h"
#include "defaults.h"


int get_default_alarm_path(cl_args* opts) {
    ssize_t pathsize = -1;
	if ((pathsize = readlink("/proc/self/exe", opts -> alarm_filename, PATH_MAX)) == -1) {
        LOG("ERROR: Couldn't get path to executable");
        return -1;
    }

    // truncate off name of executable
    for (int i = (pathsize - 1); i >= 0; i--) {
        if (opts -> alarm_filename[i] == '/') {
            opts -> alarm_filename[i] = '\0';
            break;
        }
    }

    if(strlen(opts -> alarm_filename) 
                    + strlen(DEFAULT_ALARM_FILENAME) < PATH_MAX) {
        strcat(opts -> alarm_filename, DEFAULT_ALARM_FILENAME);
    } else {
        LOG("ERROR: Default alarm file path too long");
        return -1;
    }

    return 0;
}


int get_cl_args(cl_args* opts, int argc, char* argv[]) {
    // Set defaults
    opts -> short_break_length = DEFAULT_SHORT_BREAK_LENGTH;
    opts -> long_break_length  = DEFAULT_LONG_BREAK_LENGTH;
    opts -> focus_length       = DEFAULT_FOCUS_LENGTH;
    opts -> num_short_breaks   = DEFAULT_NUM_SHORT_BREAKS;
    get_default_alarm_path(opts);

    // Check options
    int arg_counter = 1;
    while(arg_counter < argc) {
        if (strcmp(argv[arg_counter], TEST) == 0) {
            opts -> short_break_length = 2;
            opts -> long_break_length  = 3;
            opts -> focus_length       = 4;
        }

        if (strcmp(argv[arg_counter], ALARM_FILENAME) == 0
                && arg_counter + 1 < argc) {
            arg_counter++;
            if (strlen(argv[arg_counter]) < PATH_MAX) {
                strcpy(opts -> alarm_filename, argv[arg_counter]);
            } else {
                LOG("ERROR: Given alarm file path too long");
            }
        }

        arg_counter++;
    }

    return 0;
}