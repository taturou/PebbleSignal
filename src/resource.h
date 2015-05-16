#pragma once
#include <pebble.h>

enum Signal {
    Green = 0,
    Red,
    Yellow,
    // You have to modify 'MAX_SIGNAL' value.
};
typedef enum Signal Signal;
#define MAX_SIGNAL ((int)Yellow + 1)

enum OnOff {
    On = 0,
    Off,
    // You have to modify 'MAX_ONOFF' value.
};
typedef enum OnOff OnOff;
#define MAX_ONOFF ((int)Off + 1)

void resource_alloc(void);
void resource_free(void);
GBitmap *resource_get_bitmap(Signal signal, OnOff onoff);
GColor8 resource_get_color(Signal signal, OnOff onoff);