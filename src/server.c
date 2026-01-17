#include <fcntl.h>
#include <linux/limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "defaults.h"
#include "logging.h"
#include "server.h"
#include "timer.h"

typedef struct {
  int remaining_time;
  int client_fd;
  char p_state_char;
} client_thread_data;

void* client_thread_func(void* arg) {
  client_thread_data* ctd = arg;
  if (ctd == NULL) {
    LOG("ERROR: passed NULL to client_thread_func");
    return NULL;
  }

  // copy over the data
  int remaining_time = ctd->remaining_time;
  int client_fd = ctd->client_fd;
  char p_state_char = ctd->p_state_char;

  // detach the thread
  free(arg);
  pthread_detach(pthread_self());

  // only need 8 * log(2)/log(10) * sizeof(int) < 4 * sizeof(int) digits
  // to hold the base 10 string version of an int
  // +4 for p_state_char and null terminator
  char buf[4 * (sizeof(int) + 1)] = {0};

  // output consists of the p_state character
  // followed by the base 10 string of the remaining time
  buf[0] = p_state_char;
  int total_write =
      1 + snprintf(buf + 1, sizeof(buf) - 1, "%i", remaining_time);

  // loop to make sure everything gets written
  ssize_t written = 0;
  while (written < total_write) {
    ssize_t w = write(client_fd, buf + written, total_write - written);
    if (w <= 0) {
      if (errno == EINTR) {
        continue;
      } else {
        LOG_ERRNO("ERROR: write");
        break;
      }
    }
    written += w;
  }

  close(client_fd);
  return NULL;
}

server* new_server(char* socket_path) {
  server* ts = calloc(1, sizeof(server));

  if (ts == NULL) {
    LOG_ERRNO("ERROR: calloc");
    goto error_cleanup;
  }

  if (strlen(socket_path) >= sizeof((ts->addr).sun_path)) {
    LOG("ERROR: Socket path is too long");
  } else {
    strncpy((ts->addr).sun_path, socket_path, sizeof((ts->addr).sun_path) - 1);
  }

  (ts->addr).sun_family = AF_UNIX;

  ts->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (ts->server_fd == -1) {
    LOG_ERRNO("ERROR: socket");
    goto error_cleanup;
  }

  // set the server to non blocking
  int flags = fcntl(ts->server_fd, F_GETFL, 0);
  if (flags == -1) {
    LOG_ERRNO("ERROR: fcntl(F_GETFL)");
  }
  if (fcntl(ts->server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    LOG_ERRNO("ERROR: fcntl(F_SETFL)");
  }

  if (bind(ts->server_fd, (struct sockaddr* )&(ts->addr), sizeof(ts->addr)) ==
      -1) {
    LOG_ERRNO("ERROR: bind");
    goto error_cleanup;
  }

  if (chmod(socket_path, SOCKET_PERMISSIONS) == -1) {
    LOG_ERRNO("ERROR: chmod");
  }

  if (listen(ts->server_fd, SOCKET_MAX_REQUESTS) == -1) {
    LOG_ERRNO("ERROR: listen");
    goto error_cleanup;
  }

  return ts;

error_cleanup:
  del_server(ts);
  return NULL;
}

void del_server(server* ts) {
  if (ts != NULL) {
    if (ts->server_fd != -1)
      close(ts->server_fd);
    unlink((ts->addr).sun_path);
    free(ts);
  }
}

int update_server(server* ts, pomo_timer* tmr) {
  int remaining_time = get_remaining_time(tmr);

  char p_state_char = 'N';

  switch (tmr->p_state) {
  case SHORT_BREAK:
    p_state_char = 'S';
    break;
  case LONG_BREAK:
    p_state_char = 'L';
    break;
  case FOCUS:
    p_state_char = 'F';
  }

  // clear the queue
  while (true) {
    int client_fd = accept(ts->server_fd, NULL, NULL);
    if (client_fd == -1) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        LOG_ERRNO("ERROR: accept");
        break;
      }
    }

    client_thread_data* ctd =
        (client_thread_data* )malloc(sizeof(client_thread_data));
    if (!ctd) {
      LOG_ERRNO("ERROR: malloc");
      close(client_fd);
      continue;
    }

    ctd->remaining_time = remaining_time;
    ctd->client_fd = client_fd;
    ctd->p_state_char = p_state_char;

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_thread_func, ctd) != 0) {
      LOG_ERRNO("ERROR: pthread_create");
      close(client_fd);
      free(ctd);
      continue;
    }
  }

  return 0;
}