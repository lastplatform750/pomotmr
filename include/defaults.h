#pragma once

#include "typedefs.h"

// Keys for controls
// need to use define cause switch doesn't like consts
#define TOGGLE ' '
#define SKIP '\n'
#define CLEAR 'c'
#define QUIT 'q'
#define SWITCH_TASK '\t'

#define DEFAULT

// command line arguments
#define TEST "--test"

#define TASK_NAMES "--task-names"

#define ENABLE_SERVER "--enable-server"
#define ENABLE_TIMER_LOG "--enable-timer-log"
#define DISABLE_ALARM "--disable-alarm"

#define SOCKET_PATH "--socket-path"
#define ALARM_PATH "--alarm-path"
#define ERROR_PATH "--error-path"
#define TIMER_LOG_PATH "--timer-log-path"

// How many short breaks before a long break
static const uint DEFAULT_NUM_SHORT_BREAKS = 3u;

// Default lengths of the pomo sections, in seconds
// Not uints because most funcs involving time use ints
// Need to use define for the static asserts
#define DEFAULT_SHORT_BREAK_LENGTH 300
#define DEFAULT_LONG_BREAK_LENGTH 900
#define DEFAULT_FOCUS_LENGTH 1500

// The generic task name for logging if none are supplied
#define DEFAULT_TASK_NAME "Generic Task"

// (relative) path to the default alarm sound
#define DEFAULT_ALARM_PATH "resource/sample.wav"

// (relative) path for timer log
#define DEFAULT_TIMER_LOG_PATH "timer.log"

// (relative) path for error log
#define DEFAULT_ERROR_PATH "error.log"

// (global) path for the server socket
#define DEFAULT_SOCKET_PATH "/tmp/pomotmr.sock"

// maximum length for the string holding the task names
static const int MAX_LIST_STRING_LEN = 2048;

// delimiter for the task names
static const char TASK_NAMES_DELIMITER = ',';

// socket settings for the server
static const int SOCKET_PERMISSIONS = 0666;
static const int SOCKET_MAX_REQUESTS = 5;

// Sanity checks to make sure constants are good
_Static_assert(DEFAULT_FOCUS_LENGTH > 0,
               "DEFAULT_FOCUS_LENGTH must be positive");
_Static_assert(DEFAULT_LONG_BREAK_LENGTH > 0,
               "DEFAULT_LONG_BREAK_LENGTH must be positive");
_Static_assert(DEFAULT_SHORT_BREAK_LENGTH > 0,
               "DEFAULT_SHORT_BREAK_LENGTH must be positive");