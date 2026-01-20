#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defaults.h"
#include "logging.h"

error_log* open_log(int argc, char* argv[]) {
  error_log* new_log = calloc(1, sizeof(error_log));
  if (new_log == NULL) {
    LOG_ERRNO("ERROR: calloc");
    return NULL;
  }

  new_log->path = DEFAULT_ERROR_PATH;

  // Do this before the main cl argument handling
  // so that errors can be properly logged for as much as possible
  int arg_counter = 1;
  while (arg_counter < argc) {
    if (strcmp(argv[arg_counter], ERROR_PATH) && arg_counter + 1 < argc) {
      new_log->path = argv[arg_counter + 1];
      break;
    }
    arg_counter++;
  }

  new_log->filestream = freopen(new_log->path, "w", stderr);
  if (new_log->filestream == NULL) {
    LOG_ERRNO("ERROR: freopen");
    LOG("Falling back to default error path");
    new_log->path = DEFAULT_ERROR_PATH;
    new_log->filestream = freopen(new_log->path, "w", stderr);
    if (new_log->filestream == NULL) {
      LOG_ERRNO("ERROR: freopen");
      LOG("Couldn't open default error path");
      return NULL;
    }
  }

  // flush on each newline
  setvbuf(stderr, NULL, _IOLBF, 0);
  return new_log;
}

void close_log(error_log* log) {
  if (log != NULL) {
    if (log->filestream != NULL) {
      fclose(log->filestream);
    }

    struct stat st;
    if (stat(log->path, &st) == 0 && st.st_size == 0) {
      unlink(log->path);
    }

    free(log);
  }
}