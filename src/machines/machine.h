#pragma once

#ifdef KEYBOARD_HEX2A
#include "hex2a/configuration.h"
#endif

#ifdef KEYBOARD_SPLIT2040
#include "split2040/configuration.h"
#endif

// Allow keyboards without LEDs to compile properly
#ifndef LEDS_MAX
#define LEDS_MAX                (1)
#endif
#ifndef LEDS_BRIGHTNESS_DELTA
#define LEDS_BRIGHTNESS_DELTA   (1)
#endif