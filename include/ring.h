#pragma once

#include <stdatomic.h>
#include <pthread.h>

#include "typedefs.h"

typedef struct {
    int sound_filepointer;
    uint rate;
    int channels;
    atomic_bool* stop_requested;
    pthread_t ring_thread;
} ringer;

ringer* new_ringer();

void del_ringer(ringer* r);

int start_ringer(ringer* r);

int stop_ringer(ringer* r);


int play_sound(int fptr,
               uint rate,
               int channels,
               atomic_bool *stop_requested);