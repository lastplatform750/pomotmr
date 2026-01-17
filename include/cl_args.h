#pragma once

#include <linux/limits.h>

#include "typedefs.h"

#define TEST "--test"
#define ENABLE_SERVER "--server"
#define DISABLE_ALARM "--no-alarm"
#define SOCKET_PATH "-s"
#define ALARM_PATH "-a"


typedef struct {
    uint num_short_breaks;
    int short_break_length;
    int long_break_length;
    int focus_length;
    bool server_enabled;
    bool alarm_enabled;
    char* alarm_path;
    char* socket_path;
   
} cl_args;

cl_args* get_cl_args(int argc, char* argv[]);

void del_args(cl_args* opts);