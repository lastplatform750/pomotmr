#pragma once

#include <sys/un.h>
#include <time.h>

typedef enum { PLAY, PAUSE, RING } run_state;

typedef enum { SHORT_BREAK, LONG_BREAK, FOCUS } pomo_state;

typedef unsigned int uint;

typedef struct timespec timespec;

typedef struct sockaddr_un sockaddr_un;