/*
 * Simple sound playback using ALSA API and libasound.
 *
 * Compile:
 * $ cc -o play sound_playback.c -lasound
 *
 * Usage:
 * $ ./play <sample_rate> <channels> <seconds> < <file>
 *
 * Examples:
 * $ ./play 44100 2 5 < /dev/urandom
 * $ ./play 22050 1 8 < /path/to/file.wav
 *
 * Copyright (C) 2009 Alessandro Ghedini <al3xbio@gmail.com>
 * --------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Alessandro Ghedini wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * --------------------------------------------------------------
 */

// Modified by me

#include <alsa/asoundlib.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "error_log.h"
#include "ring.h"

#define PCM_DEVICE "default"
#define WAV_FILENAME "./resource/sample.wav"

const int DEFAULT_RATE = 44100;
const int DEFAULT_NUM_CHANNELS = 2;

void* ring_thread_func(void* arg) {
  ringer* r = arg;

  if (r == NULL) {
    LOG("ERROR: Passed NULL to ring_thread_func");
    return NULL;
  }

  int alarm_fptr = open(r->alarm_filename, O_RDONLY);

  if (alarm_fptr == -1) {
    LOG_ERRNO("ERROR: Couldn't open \"%s\"", r->alarm_filename);
    return NULL;
  }

  uint tmp;
  int pcm;
  snd_pcm_hw_params_t* params;
  snd_pcm_uframes_t frames;
  char* buff;
  int buff_size;

  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(r->pcm_handle, params);

  /* Set parameters */
  if ((pcm = snd_pcm_hw_params_set_access(r->pcm_handle, params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    LOG("ERROR: Can't set interleaved mode. %s", snd_strerror(pcm));

  if ((pcm = snd_pcm_hw_params_set_format(r->pcm_handle, params,
                                          SND_PCM_FORMAT_S16_LE)) < 0)
    LOG("ERROR: Can't set format. %s", snd_strerror(pcm));

  if ((pcm = snd_pcm_hw_params_set_channels(r->pcm_handle, params,
                                            r->channels)) < 0)
    LOG("ERROR: Can't set channels number. %s", snd_strerror(pcm));

  if ((pcm = snd_pcm_hw_params_set_rate_near(r->pcm_handle, params, &(r->rate),
                                             0)) < 0)
    LOG("ERROR: Can't set rate. %s", snd_strerror(pcm));

  /* Write parameters */
  if ((pcm = snd_pcm_hw_params(r->pcm_handle, params)) < 0)
    LOG("ERROR: Can't set hardware parameters. %s", snd_strerror(pcm));

  snd_pcm_hw_params_get_period_size(params, &frames, 0);
  buff_size = frames * r->channels * 2; // 2 bytes/sample for S16_LE
  buff = (char* )calloc(1, buff_size);
  if (buff == NULL) {
    LOG_ERRNO("ERROR: calloc");
    goto cleanup;
  }

  snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

  // seek to beginning of file
  if (lseek(alarm_fptr, 0, SEEK_SET) == -1) {
    LOG_ERRNO("ERROR: lseek");
    goto cleanup;
  }

  // Play until EOF or stop_requested is set.
  while (!atomic_load(r->stop_requested)) {
    ssize_t read_size = read(alarm_fptr, buff, buff_size);
    if (read_size == 0) {
      // EOF: rewind to start to loop the file, or break to stop
      if (lseek(alarm_fptr, 0, SEEK_SET) == -1)
        break;
      continue;
    } else if (read_size < 0) {
      LOG_ERRNO("ERROR: read");
      break;
    }

    // write (one period) to ALSA
    pcm = snd_pcm_writei(r->pcm_handle, buff, frames);
    if (pcm == -EPIPE) {
      // underrun
      snd_pcm_prepare(r->pcm_handle);
    } else if (pcm < 0) {
      LOG("ERROR: Can't write to PCM device: %s", snd_strerror(pcm));
      break;
    }

    // At this point we've written one buffer (period). Check stop again
    if (atomic_load(r->stop_requested)) {
      // drop immediately (do not drain) so audio cuts off
      snd_pcm_drop(r->pcm_handle);
      break;
    }
  }

cleanup:
  if (alarm_fptr != -1)
    close(alarm_fptr);

  if (!atomic_load(r->stop_requested))
    snd_pcm_drain(r->pcm_handle);

  if (buff != NULL)
    free(buff);

  return NULL;
}

ringer* new_ringer(char* alarm_filename) {
  ringer* new_r = NULL;

  new_r = (ringer* )calloc(1, sizeof(ringer));
  if (new_r == NULL) {
    LOG_ERRNO("ERROR: calloc");
    goto error_cleanup;
  }

  new_r->stop_requested = (atomic_bool* )calloc(1, sizeof(atomic_bool));
  if (new_r->stop_requested == NULL) {
    LOG_ERRNO("ERROR: calloc");
    goto error_cleanup;
  }

  atomic_init(new_r->stop_requested, false);

  new_r->rate = DEFAULT_RATE;
  new_r->channels = DEFAULT_NUM_CHANNELS;
  new_r->alarm_filename = alarm_filename;

  /* Open the PCM device in playback mode */
  int pcm;
  if ((pcm = snd_pcm_open(&(new_r->pcm_handle), PCM_DEVICE,
                          SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    LOG("ERROR: Can't open \"%s\" PCM device: %s", PCM_DEVICE,
        snd_strerror(pcm));
    goto error_cleanup;
  }

  return new_r;

error_cleanup:
  del_ringer(new_r);
  return NULL;
}

void del_ringer(ringer* r) {
  if (r != NULL) {
    if ((r->stop_requested) != NULL)
      free(r->stop_requested);
    if ((r->pcm_handle) != NULL) {
      snd_pcm_close(r->pcm_handle);
    }
    free(r);
  }
}

int start_ringer(ringer* r) {
  atomic_store(r->stop_requested, false);
  pthread_create(&(r->ring_thread), NULL, ring_thread_func, r);

  return 0;
}

int stop_ringer(ringer* r) {
  atomic_store(r->stop_requested, true);
  pthread_join(r->ring_thread, NULL);

  return 0;
}
