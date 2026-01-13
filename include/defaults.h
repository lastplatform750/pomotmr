#pragma once

#include "typedefs.h"

// How many short breaks before a long break
static const uint DEFAULT_NUM_SHORT_BREAKS   = 3u;

// Default lengths of the pomo sections
// Not uints because most funcs involving time use ints
// Need to use define for the static asserts
#define DEFAULT_SHORT_BREAK_LENGTH 300
#define DEFAULT_LONG_BREAK_LENGTH  900
#define DEFAULT_FOCUS_LENGTH       1500

// Keys for controls
// need to use define cause switch doesn't like consts
#define TOGGLE ' '
#define SKIP   '\n'
#define CLEAR  'c'


// Sanity checks to make sure constants are good
_Static_assert(DEFAULT_FOCUS_LENGTH > 0,
                "DEFAULT_FOCUS_LENGTH must be positive");
_Static_assert(DEFAULT_LONG_BREAK_LENGTH > 0, 
                "DEFAULT_LONG_BREAK_LENGTH must be positive");
_Static_assert(DEFAULT_SHORT_BREAK_LENGTH > 0,
                "DEFAULT_SHORT_BREAK_LENGTH must be positive");