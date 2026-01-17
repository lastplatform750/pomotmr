#pragma once

#include <errno.h>
#include <stdio.h>
#include <string.h>

// Note: in all of these msg has to be a string literal

#define LOG_ERRNO(msg, ...)                                                    \
  fprintf(stderr, "[%s, %d]: " msg ": %s\n", __FILE__,                         \
          __LINE__ __VA_OPT__(, ) __VA_ARGS__, strerror(errno))

#define LOG(msg, ...)                                                          \
  fprintf(stderr, "[%s, %d]: " msg "\n", __FILE__,                             \
          __LINE__ __VA_OPT__(, ) __VA_ARGS__)
