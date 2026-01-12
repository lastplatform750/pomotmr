#pragma once

#include "typedefs.h"

#include <stdatomic.h>
#include <pthread.h>

typedef struct {
    const char *filename;
    uint rate;
    int channels;
    atomic_bool* stop_requested;
    pthread_t ring_thread;
} ringer;

ringer* new_ringer();

void del_ringer(ringer* r);

int start_ringer(ringer* r);

int stop_ringer(ringer* r);


int play_sound(const char *filename,
               uint rate,
               int channels,
               atomic_bool *stop_requested);