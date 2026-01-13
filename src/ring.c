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
#include <linux/limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "ring.h"

#define PCM_DEVICE "default"
#define WAV_FILENAME "./resource/sample.wav"

const int DEFAULT_RATE         = 44100;
const int DEFAULT_NUM_CHANNELS = 2;


void* ring_thread_func(void* arg) {
	ringer* r = arg;

	if (r == NULL) {
		fprintf(stderr, "ERROR: Passed NULL to ring_thread_func\n");
		return NULL;
	}

	while (!atomic_load(r->stop_requested)) {
        play_sound(r -> sound_filepointer, 
                    r -> rate,
                    r -> channels, 
                    r -> stop_requested);
    }

    return NULL;
}


ringer* new_ringer() {
	ringer* new_r = NULL;

	char buf[PATH_MAX];
    ssize_t pathsize = -1;

	int fptr = -1;

	new_r = (ringer*) malloc(sizeof(ringer));
	if (new_r == NULL) {
		perror("ERROR: malloc");
		goto error_cleanup;
	}

	new_r -> stop_requested = (atomic_bool*) malloc(sizeof(atomic_bool));
	if (new_r -> stop_requested == NULL) {
		perror("ERROR: malloc");
		goto error_cleanup;
	} 

	if ((pathsize = readlink("/proc/self/exe", buf, PATH_MAX) == -1)) {
        fprintf(stderr, "ERROR: Couldn't get path to executable\n");
        goto error_cleanup;
    }

	int len = strlen(buf);
	buf[len - 7] = '\0'; // remove name of executable
	strcat(buf, "resource/sample.wav");

	fprintf(stderr, "file: %s\n", buf);

	if ((fptr = open(buf, O_RDONLY)) == -1) {
        fprintf(stderr, "ERROR: Can't open \"%s\"\n",
                    buf);
        goto error_cleanup;
    }

	atomic_init(new_r -> stop_requested, false);

	new_r -> rate     = DEFAULT_RATE;
	new_r -> channels = DEFAULT_NUM_CHANNELS;
	new_r -> sound_filepointer = fptr;

	return new_r;

	error_cleanup:
		if (!(new_r -> stop_requested)) free(new_r -> stop_requested);
		if (!new_r) free(new_r);
		if (fptr != -1) close(fptr);
		
		return NULL;
}


void del_ringer(ringer* r) {
	if (!r) {
		if (!(r -> stop_requested)) free(r -> stop_requested);
		if (r -> sound_filepointer != -1) close(r -> sound_filepointer);
		free(r);
	}
}


int start_ringer(ringer* r) {
	atomic_store(r -> stop_requested, false);
    pthread_create(&(r -> ring_thread), 
                    NULL, 
                    ring_thread_func, 
                    r);

	return 0;
}


int stop_ringer(ringer* r) {
	atomic_store(r -> stop_requested, true);
	pthread_join(r -> ring_thread, NULL);

	return 0;
}


int play_sound(int fptr,
               unsigned int rate,
               int channels,
               atomic_bool* stop_requested) {
	uint tmp;
    int pcm;
	snd_pcm_t* pcm_handle;
	snd_pcm_hw_params_t* params;
	snd_pcm_uframes_t frames;
	char* buff;
	int buff_size;

    if (fptr == -1) {
        fprintf(stderr, "ERROR: Invalid filepointer\n");
        return -1;
    }

	/* Open the PCM device in playback mode */
	if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
		fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	if ((pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
		fprintf(stderr, "ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
						SND_PCM_FORMAT_S16_LE)) < 0) 
		fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels)) < 0) 
		fprintf(stderr, "ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0)) < 0) 
		fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(pcm));

	/* Write parameters */
	if ((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0)
		fprintf(stderr, "ERROR: Can't set hardware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
	//printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
	//printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
	//snd_pcm_hw_params_get_channels(params, &tmp);
	//printf("channels: %i ", tmp);
	//if (tmp == 1)
		//printf("(mono)\n");
	//else if (tmp == 2)
		//printf("(stereo)\n");
	//snd_pcm_hw_params_get_rate(params, &tmp, 0);
	//printf("rate: %d bps\n", tmp);
	//printf("seconds: %d\n", seconds);	

	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	buff_size = frames * channels * 2; // 2 bytes/sample for S16_LE
    buff = (char*) malloc(buff_size);
    if (buff == NULL) {
        perror("ERROR: malloc");
        goto cleanup;
    }

    snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
    // loops = (seconds * 1000000) / tmp;  // old approach with seconds

	// seek to beginning of file
	if (lseek(fptr, 0, SEEK_SET) == -1) {
		perror("ERROR: lseek");
	}

    // Play until EOF or stop_requested is set.
    while (!atomic_load(stop_requested)) {
        ssize_t r = read(fptr, buff, buff_size);
        if (r == 0) {
            // EOF: rewind to start to loop the file, or break to stop
            if (lseek(fptr, 0, SEEK_SET) == -1) break;
            continue;
        } else if (r < 0) {
            perror("ERROR: read");
            break;
        }

        // write (one period) to ALSA
        pcm = snd_pcm_writei(pcm_handle, buff, frames);
        if (pcm == -EPIPE) {
            // underrun
            snd_pcm_prepare(pcm_handle);
        } else if (pcm < 0) {
            fprintf(stderr, "ERROR: Can't write to PCM device: %s\n",
                    snd_strerror(pcm));
					break;
        }

        // At this point we've written one buffer (period). Check stop again
        if (atomic_load(stop_requested)) {
            // drop immediately (do not drain) so audio cuts off
            snd_pcm_drop(pcm_handle);
            break;
        }
    }

	cleanup:
    	if (pcm_handle) {
        	// If we didn't call snd_pcm_drop already and stop wasn't requested, do a normal drain.
        	if (!atomic_load(stop_requested)) snd_pcm_drain(pcm_handle);
        	snd_pcm_close(pcm_handle);
        	pcm_handle = NULL;
    	}
    	if (buff) free(buff);

		return 0;
}
