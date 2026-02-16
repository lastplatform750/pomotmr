#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cl_args.h"
#include "defaults.h"
#include "error_log.h"

char* init_field(const char* source, int max_len) {
  char* ret = NULL;
  ssize_t source_len = strlen(source);
  if (source_len >= max_len) {
    LOG("ERROR: String \"%s\" is too long", source);
    return NULL;
  } else {
    ret = calloc(source_len + 1, sizeof(char));
    if (ret == NULL) {
      LOG_ERRNO("ERROR: calloc");
      return NULL;
    }
    strncpy(ret, source, source_len);
  }

  return ret;
}

char* get_relative_default_path(const char* relative_path) {
  char buf[PATH_MAX] = {0};

  ssize_t pathsize = -1;
  if ((pathsize = readlink("/proc/self/exe", buf, PATH_MAX)) == -1) {
    LOG_ERRNO("ERROR: readlink");
    LOG("ERROR: Couldn't get path to executable");
    return NULL;
  }

  // truncate off name of executable
  for (int i = (pathsize - 1); i >= 0; i--) {
    if (buf[i] == '/') {
      buf[i + 1] = '\0';
      break;
    }
  }

  ssize_t total_len = strlen(buf) + strlen(relative_path);

  if (total_len >= PATH_MAX) {
    LOG("ERROR: File path too long: \"%s%s\"", buf, relative_path);
    return NULL;
  } else {
    strcat(buf, relative_path);
  }

  return init_field(buf, PATH_MAX);
}

cl_args* get_cl_args(int argc, char* argv[]) {
  cl_args* opts = calloc(1, sizeof(cl_args));
  if (opts == NULL) {
    LOG_ERRNO("ERROR: calloc");
    return NULL;
  }

  opts->alarm_path = NULL;
  opts->socket_path = NULL;
  opts->timer_log_path = NULL;

  // Set defaults
  opts->short_break_length = DEFAULT_SHORT_BREAK_LENGTH;
  opts->long_break_length = DEFAULT_LONG_BREAK_LENGTH;
  opts->focus_length = DEFAULT_FOCUS_LENGTH;
  opts->num_short_breaks = DEFAULT_NUM_SHORT_BREAKS;
  opts->server_enabled = false;
  opts->timer_log_enabled = false;
  opts->alarm_enabled = true;

  // Check options
  int arg_counter = 1;
  while (arg_counter < argc) {
    if (strcmp(argv[arg_counter], TEST) == 0) {
      opts->short_break_length = 2;
      opts->long_break_length = 3;
      opts->focus_length = 4;
    }

    if (strcmp(argv[arg_counter], ENABLE_SERVER) == 0) {
      opts->server_enabled = true;
    }

    if (strcmp(argv[arg_counter], ENABLE_TIMER_LOG) == 0) {
      opts->timer_log_enabled = true;
    }

    if (strcmp(argv[arg_counter], DISABLE_ALARM) == 0) {
      opts->alarm_enabled = false;
    }

    if (strcmp(argv[arg_counter], ALARM_PATH) == 0 && arg_counter + 1 < argc &&
        opts->alarm_path == NULL) {
      arg_counter++;
      opts->alarm_path = init_field(argv[arg_counter], PATH_MAX);
      if (opts->alarm_path == NULL) {
        LOG("ERROR: Couldn't get given alarm path: \"%s\"", argv[arg_counter]);
      }
    }

    if (strcmp(argv[arg_counter], SOCKET_PATH) == 0 && arg_counter + 1 < argc &&
        opts->socket_path == NULL) {
      arg_counter++;
      opts->socket_path = init_field(argv[arg_counter], PATH_MAX);
      if (opts->alarm_path == NULL) {
        LOG("ERROR: Couldn't get given socket path: \"%s\"", argv[arg_counter]);
      }
    }

    if (strcmp(argv[arg_counter], TIMER_LOG_PATH) == 0 &&
        arg_counter + 1 < argc && opts->timer_log_path == NULL) {
      arg_counter++;
      opts->timer_log_path = init_field(argv[arg_counter], PATH_MAX);
      if (opts->timer_log_path == NULL) {
        LOG("ERROR: Couldn't get given timer log path: \"%s\"",
            argv[arg_counter]);
      }
    }

    arg_counter++;
  }

  if (opts->alarm_enabled == true && opts->alarm_path == NULL) {
    opts->alarm_path = get_relative_default_path(DEFAULT_ALARM_PATH);
    if (opts->alarm_path == NULL) {
      LOG("ERROR: Couldn't get default alarm path, disabling alarm");
      opts->alarm_enabled = false;
    }
  }

  if (opts->timer_log_enabled == true && opts->timer_log_path == NULL) {
    opts->timer_log_path = get_relative_default_path(DEFAULT_TIMER_LOG_PATH);
    if (opts->timer_log_path == NULL) {
      LOG("ERROR: Couldn't get default timer log path, disabling timer log");
      opts->timer_log_enabled = false;
    }
  }

  if (opts->server_enabled == true && opts->socket_path == NULL) {
    opts->socket_path = init_field(DEFAULT_SOCKET_PATH, PATH_MAX);
    if (opts->socket_path == NULL) {
      LOG("ERROR: Couldn't get default socket path, disabling server");
      opts->server_enabled = false;
    }
  }

  return opts;
}

void del_args(cl_args* opts) {
  if (opts != NULL) {
    if (opts->alarm_path != NULL)
      free(opts->alarm_path);
    if (opts->timer_log_path != NULL)
      free(opts->timer_log_path);
    if (opts->socket_path != NULL)
      free(opts->socket_path);
    free(opts);
  }
}
