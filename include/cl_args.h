#pragma once

#include <linux/limits.h>

#include "typedefs.h"

#define TEST "--test"
#define ALARM_FILENAME "-a"

#define DEFAULT_ALARM_FILENAME "resource/sample.wav"

typedef struct {
    uint num_short_breaks;
    int short_break_length;
    int long_break_length;
    int focus_length;
    char alarm_filename[PATH_MAX];
} cl_args;

int get_cl_args(cl_args* ca, int argc, char* argv[]);