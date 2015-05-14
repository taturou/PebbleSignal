#include <pebble.h>
#include "timebar_layer.h"
    
static Window *s_window;
static BitmapLayer *s_bmp_layer;
static TimebarLayer *s_timebar_layer[2];
static GBitmap *s_bmp[2];    // 0:green, 1:red
static bool is_tick_timer_subscribe = false;

#define GREEN    (0)
#define RED      (1)

#define MIN      (0)
#define SEC      (1)

static void s_update(int green_or_red, bool hidden) {
    Layer *layer = bitmap_layer_get_layer(s_bmp_layer);

    if (hidden == true) {
        if (layer_get_hidden(layer) == false) {
            layer_set_hidden(layer, true);
        }
    } else {
        if (layer_get_hidden(layer) == true) {
            layer_set_hidden(layer, false);
        }

        if (green_or_red == GREEN) {
            if (bitmap_layer_get_bitmap(s_bmp_layer) == s_bmp[RED]) {
                bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[GREEN]);
            }
        } else {
            if (bitmap_layer_get_bitmap(s_bmp_layer) == s_bmp[GREEN]) {
                bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[RED]);
            }
        }
    }
}
    
static void s_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "%02d:%02d:%02d", tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);

    if ((tick_time->tm_min % 2) == 0) {
        s_update(RED, false);
        
        timebar_layer_set_bar_color(s_timebar_layer[MIN], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_color(s_timebar_layer[SEC], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_height(s_timebar_layer[MIN], 1);
        timebar_layer_set_bar_height(s_timebar_layer[SEC], 1);
        timebar_layer_set_hidden(s_timebar_layer[SEC], false);
    } else {
        if (tick_time->tm_sec < 40) {
            s_update(GREEN, false);

            timebar_layer_set_hidden(s_timebar_layer[SEC], false);
        } else {
            if ((tick_time->tm_sec % 2) == 0) {
                s_update(GREEN, false);
            } else {
                s_update(GREEN, true);
            }
            timebar_layer_set_hidden(s_timebar_layer[SEC], true);
        }

        timebar_layer_set_bar_color(s_timebar_layer[MIN], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_color(s_timebar_layer[SEC], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_height(s_timebar_layer[MIN], 1);
        timebar_layer_set_bar_height(s_timebar_layer[SEC], 2);
    }

    timebar_layer_set_value(s_timebar_layer[MIN], tick_time->tm_min);
    timebar_layer_set_value(s_timebar_layer[SEC], tick_time->tm_sec);
}
    
static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (is_tick_timer_subscribe == true) {
        tick_timer_service_unsubscribe();
        s_update(GREEN, false);
        is_tick_timer_subscribe = false;
    } else {
        s_update(GREEN, false);
        tick_timer_service_subscribe(SECOND_UNIT, s_tick_handler);
        is_tick_timer_subscribe = true;
    }
}

static void s_up_click_handler(ClickRecognizerRef recognizer, void *context) {
    Layer *layer = bitmap_layer_get_layer(s_bmp_layer);
    
    if (layer_get_hidden(layer) == true) {
        layer_set_hidden(layer, false);
    } else {
        layer_set_hidden(layer, true);
    }
}

static void s_down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (bitmap_layer_get_bitmap(s_bmp_layer) == s_bmp[GREEN]) {
        bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[RED]);
    } else {
        bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[GREEN]);
    }
}

static void s_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, s_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, s_down_click_handler);
}

static void s_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    // create bitmap
    s_bmp[GREEN] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_GREEN);
    s_bmp[RED] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_RED);

    // create bitmap-layer
    s_bmp_layer = bitmap_layer_create(window_bounds);
    bitmap_layer_set_background_color(s_bmp_layer, GColorBlack);
    bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[GREEN]);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bmp_layer));

    // create time-layer
    s_timebar_layer[MIN] = timebar_layer_create(GPoint(5, 26));
    layer_add_child(window_layer, timebar_layer_get_layer(s_timebar_layer[MIN]));

    s_timebar_layer[SEC] = timebar_layer_create(GPoint(124, 26));
    layer_add_child(window_layer, timebar_layer_get_layer(s_timebar_layer[SEC]));
    
    // start tick-timer
    s_update(GREEN, false);
    tick_timer_service_subscribe(SECOND_UNIT, s_tick_handler);
    is_tick_timer_subscribe = true;
}

static void s_window_unload(Window *window) {
    timebar_layer_destroy(s_timebar_layer[MIN]);
    timebar_layer_destroy(s_timebar_layer[SEC]);
    
    bitmap_layer_destroy(s_bmp_layer);
    
    gbitmap_destroy(s_bmp[GREEN]);
    gbitmap_destroy(s_bmp[RED]);
}

static void s_init(void) {
    s_window = window_create();
#if PBL_PLATFORM_APLITE
    window_set_fullscreen(s_window, true);
#endif /* PBL_PLATFORM_APLITE */
    window_set_background_color(s_window, GColorBlack);
    window_set_click_config_provider(s_window, s_click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = s_window_load,
        .unload = s_window_unload,
    });
    const bool animated = false;
    window_stack_push(s_window, animated);
}

static void s_deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    s_init();
    app_event_loop();
    s_deinit();
}