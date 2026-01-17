#pragma once

#include <pthread.h>
#include <stdatomic.h>

#include "typedefs.h"

typedef struct {
  char* alarm_filename;
  uint rate;
  int channels;
  atomic_bool* stop_requested;
  pthread_t ring_thread;
} ringer;

ringer* new_ringer(char* alarm_filename);

void del_ringer(ringer* r);

int start_ringer(ringer* r);

int stop_ringer(ringer* r);

int play_sound(int fptr, uint rate, int channels, atomic_bool* stop_requested);