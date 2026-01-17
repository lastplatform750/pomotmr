#include <linux/limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defaults.h"
#include "logging.h"
#include "timer.h"
#include "server.h"

typedef struct {
    int remaining_time;
    int client_fd;
} client_thread_data;


void* client_thread_func(void* arg) {
    client_thread_data* ctd = arg;
    if (ctd == NULL) {
        LOG("ERROR: passed NULL to client_thread_func");
        return NULL;
    }

    int remaining_time = ctd -> remaining_time;
    int client_fd = ctd -> client_fd;
    free(arg);
    pthread_detach(pthread_self());

    char buf[256];

    int total_write = snprintf(buf, sizeof(buf), "%i", remaining_time);
    
    // loop to make sure everything gets written
    ssize_t written = 0;
    while (written < total_write) {
        ssize_t w = write(client_fd, buf + written, total_write - written);
        if (w <= 0) {
            if (errno == EINTR) continue;
            break;
        }
        written += w;
    }

    close(client_fd);
    return NULL;
}


void* server_thread_func(void* arg) {
    server* ts = (server*) arg;
    if (ts == NULL) {
        LOG("ERROR: passed NULL to server_thread_func");
        return NULL;
    }

    while(!atomic_load(ts -> stop_requested)) {
        int client_fd = accept(ts -> server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EINTR
                || errno == EAGAIN
                || errno == EWOULDBLOCK) {
                    sleep(1);
                    continue;
            }
            LOG_ERRNO("ERROR: accept");
            break;
        }

        client_thread_data* ctd = malloc(sizeof(client_thread_data));
        if (!ctd) {
            LOG_ERRNO("ERROR: malloc");
            close(client_fd);
            continue;
        }
        
        ctd -> remaining_time = atomic_load(ts -> remaining_time);
        ctd -> client_fd = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread,
                            NULL, 
                            client_thread_func, 
                            ctd) != 0) {
            LOG_ERRNO("ERROR: pthread_create");
            close(client_fd);
            free(ctd);
            continue;
        }
    }

    return NULL;
}


server* new_server(char* socket_path) {
    server* ts = calloc(1, sizeof(server));

    if (ts == NULL) {
        LOG_ERRNO("ERROR: calloc");
        goto error_cleanup;
    }

    if (strlen(socket_path) >= sizeof((ts -> addr).sun_path)) {
        LOG("ERROR: Socket path is too long");
    } else {
        strncpy((ts -> addr).sun_path, socket_path, sizeof((ts -> addr).sun_path)-1);
    }

    (ts -> addr).sun_family = AF_UNIX;

    ts -> server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (ts -> server_fd == -1) {
        LOG_ERRNO("ERROR: socket");
        goto error_cleanup;
    }

    // set the server to non blocking
    int flags = fcntl(ts -> server_fd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERRNO("ERROR: fcntl");
    }
    if (fcntl(ts -> server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERRNO("fcntl");
    }

    if (bind(ts -> server_fd, 
            (struct sockaddr*) &(ts -> addr), 
            sizeof(ts -> addr)) == -1) {
        LOG_ERRNO("ERROR: bind");
        goto error_cleanup;
    }

    if (chmod(socket_path, SOCKET_PERMISSIONS) == -1) {
        LOG_ERRNO("ERROR: chmod");
    }

    if (listen(ts -> server_fd, SOCKET_MAX_REQUESTS) == -1) {
        LOG_ERRNO("ERROR: listen");
        goto error_cleanup;
    }

    ts -> stop_requested = (atomic_bool*) malloc(sizeof(atomic_bool));
	if (ts -> stop_requested == NULL) {
		LOG_ERRNO("ERROR: malloc");
		goto error_cleanup;
	}

    ts -> remaining_time = (atomic_int*) malloc(sizeof(atomic_int));
    if (ts -> remaining_time == NULL) {
        LOG_ERRNO("ERROR: malloc");
        goto error_cleanup;
    }

    atomic_init(ts -> stop_requested, false);
    atomic_init(ts -> remaining_time, 0);

    return ts;

    error_cleanup:
            del_server(ts);
            return NULL;
}


void del_server(server* ts) {
    if (ts != NULL) {
        if ((ts -> stop_requested) != NULL) free(ts -> stop_requested);
        if ((ts -> remaining_time) != NULL) free(ts -> remaining_time);
        if (ts -> server_fd != -1) close(ts -> server_fd);
        unlink((ts -> addr).sun_path);
        free(ts);
    }
}


int start_server(server* ts) {
    if (pthread_create(&(ts -> server_thread),
    NULL,
    server_thread_func,
    ts) != 0) {
        LOG("ERROR: pthread_create");
        return -1;
    }

    return 0;
}


int stop_server(server* ts) {
    atomic_store(ts -> stop_requested, true);
	pthread_join(ts -> server_thread, NULL);

	return 0;
}


int update_server(server* ts, pomo_timer* tmr) {
    atomic_store(ts -> remaining_time, get_remaining_time(tmr));
    return 0;
}