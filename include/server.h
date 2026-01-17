#pragma once

#include <linux/limits.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdatomic.h>


#include "timer.h"
#include "typedefs.h"

typedef struct {
    sockaddr_un addr;
    int server_fd;
} server;

server* new_server(char* socket_path);

void del_server(server* ts);

int update_server(server* ts, pomo_timer* tmr);
