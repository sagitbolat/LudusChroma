#pragma once
#include <stdint.h>
#include "sky_structs.h"

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif


// NOTE: Services that the game provides to the platform layer:
    // TODO: Maybe seperate sound on another thread and update async.

// NOTE: We need timing, controller/keyboard input, bitmap buffer for graphics, sound buffer.

static void GameUpdateAndRender(Vector2Int screen_size, GameMemory*, KeyboardState*, double delta_time);
