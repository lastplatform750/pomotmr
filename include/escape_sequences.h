#pragma once

#include <string.h>

#define ENABLE_ALT_BUF "\x1b[?1049h"
#define DISABLE_ALT_BUF "\x1b[?1049l"
#define SHOW_CURSOR "\x1b[?25h"
#define HIDE_CURSOR "\x1b[?25l"
#define SAVE_SCREEN "\x1b[?47h"
#define RESTORE_SCREEN "\x1b[?47l"
#define CLEAR "\x1b[2J"
#define HOME "\x1b[H"

const size_t HOME_LEN = strlen(HOME);