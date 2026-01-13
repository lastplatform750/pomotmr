#pragma once

#include "typedefs.h"

// How many short breaks before a long break
static const uint DEFAULT_NUM_SHORT_BREAKS   = 3u;

// Default lengths of the pomo sections
static const uint DEFAULT_SHORT_BREAK_LENGTH = 300u;
static const uint DEFAULT_LONG_BREAK_LENGTH  = 900u;
static const uint DEFAULT_FOCUS_LENGTH       = 1500u;

// Keys for controls - need to use define cause switch doesn't like consts
#define TOGGLE ' '
#define SKIP   '\n'
#define CLEAR  'c'