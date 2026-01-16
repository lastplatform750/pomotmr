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
    pthread_t server_thread;
    atomic_bool* stop_requested;
    atomic_int* remaining_time;
} server;

server* new_server(char* socket_path);

int start_server(server* ts);

void del_server(server* ts);

int stop_server(server* ts);

int update_server(server* ts, pomo_timer* tmr);
