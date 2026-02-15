#pragma once

#include <linux/limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/un.h>

#include "timer.h"
#include "typedefs.h"
#include "cl_args.h"

typedef struct {
  sockaddr_un addr;
  int server_fd;
} server;

server* new_server(cl_args* opts);

void del_server(server* ts);

int update_server(server* ts, pomo_timer* tmr);
