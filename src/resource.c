#include <pebble.h>
#include "resource.h"

static GBitmap *s_bitmap[MAX_SIGNAL][MAX_ONOFF];
static GColor8 s_color[MAX_SIGNAL][MAX_ONOFF];

void resource_alloc(void) {
    // bitmap
    s_bitmap[Green][On] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_GREEN_JAEGERGREEN);
    s_bitmap[Green][Off] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_GREEN_DARKGRAY);
    s_bitmap[Red][On] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_RED_RED);
    s_bitmap[Red][Off] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_RED_DARKGRAY);
    s_bitmap[Yellow][On] = NULL;
    s_bitmap[Yellow][Off] = NULL;
    
    // color
    s_color[Green][On] = GColorJaegerGreen;
    s_color[Green][Off] = GColorDarkGray;
    s_color[Red][On] = GColorRed;
    s_color[Red][Off] = GColorDarkGray;
    s_color[Yellow][On] = GColorChromeYellow;
    s_color[Yellow][Off] = GColorDarkGray;
}

void resource_free(void) {
    for (int s = 0; s < MAX_SIGNAL; s++) {
        for(int o = 0; o < MAX_ONOFF; o++) {
            if (s_bitmap[s][o] != NULL) {
                gbitmap_destroy(s_bitmap[s][o]);
            }
        }
    }
}

GBitmap *resource_get_bitmap(Signal signal, OnOff onoff) {
    return s_bitmap[signal][onoff];
}

GColor8 resource_get_color(Signal signal, OnOff onoff) {
    return s_color[signal][onoff];
}