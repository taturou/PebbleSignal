#include <pebble.h>
#include "timebar_layer.h"
#include "time_layer.h"

/* configurations */
#define DISPLAY_TIMEBAR_HOUR_AND_MIN    (0)

/* code */
static Window *s_window;
static TimeLayer *s_time_layer;
static Layer *s_base_layer;
static BitmapLayer *s_bmp_layer;
static TimebarLayer *s_timebar_layer[2];
static GBitmap *s_bmp[2];    // 0:green, 1:red
static bool is_tick_timer_subscribe = false;
static bool is_display_time = false;

#define GREEN    (0)
#define RED      (1)

#define MIN      (0)
#define SEC      (1)

#define DELAY_UP_TO_DOWN    (4000)
    
static void s_base_layer_update_proc(struct Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

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
#if DISPLAY_TIMEBAR_HOUR_AND_MIN
    if ((tick_time->tm_min % 2) == 0) {
        if (tick_time->tm_sec < 59) {
            s_update(RED, false);
        } else {
            s_update(RED, true);
        }
        timebar_layer_set_bar_color(s_timebar_layer[MIN], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_color(s_timebar_layer[SEC], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_height(s_timebar_layer[MIN], 1);
        timebar_layer_set_bar_height(s_timebar_layer[SEC], 1);
    } else {
        if (tick_time->tm_sec < 40) {
            s_update(GREEN, false);

        } else {
            if ((tick_time->tm_sec % 2) == 0) {
                s_update(GREEN, false);
            } else {
                s_update(GREEN, true);
            }
        }
        timebar_layer_set_bar_color(s_timebar_layer[MIN], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_color(s_timebar_layer[SEC], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_height(s_timebar_layer[MIN], 1);
        timebar_layer_set_bar_height(s_timebar_layer[SEC], 2);
    }
    timebar_layer_set_value(s_timebar_layer[MIN], tick_time->tm_min);
    timebar_layer_set_value(s_timebar_layer[SEC], tick_time->tm_sec);
#else /* ! DISPLAY_TIMEBAR_HOUR_AND_MIN */
    if ((tick_time->tm_min % 2) == 0) {
        if (tick_time->tm_sec < 59) {
            s_update(RED, false);
        } else {
            s_update(RED, true);
        }
        timebar_layer_set_bar_color(s_timebar_layer[0], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_color(s_timebar_layer[1], GColorRed, GColorBulgarianRose);
        timebar_layer_set_bar_height(s_timebar_layer[0], 5);
        timebar_layer_set_bar_height(s_timebar_layer[1], 5);
    } else {
        if (tick_time->tm_sec < 45) {
            s_update(GREEN, false);

        } else {
            if ((tick_time->tm_sec % 2) == 0) {
                s_update(GREEN, false);
            } else {
                s_update(GREEN, true);
            }
        }
        timebar_layer_set_bar_color(s_timebar_layer[0], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_color(s_timebar_layer[1], GColorJaegerGreen, GColorMidnightGreen);
        timebar_layer_set_bar_height(s_timebar_layer[0], 7);
        timebar_layer_set_bar_height(s_timebar_layer[1], 7);
    }
    timebar_layer_set_value(s_timebar_layer[0], tick_time->tm_sec / 3);
    timebar_layer_set_value(s_timebar_layer[1], tick_time->tm_sec / 3);
#endif /* ! DISPLAY_TIMEBAR_HOUR_AND_MIN */
}

static void s_down_animation_stopped_handler(Animation *down_animation, bool finished, void *context) {
    animation_destroy(down_animation);
    time_layer_set_hidden(s_time_layer, true);
    is_display_time = false;
}

static void s_up_animation_stopped_handler(Animation *up_animation, bool finished, void *context) {
    animation_destroy(up_animation);

    // start animation to hide
    GRect from_frame = layer_get_frame(s_base_layer);
    GRect to_frame = GRect(from_frame.origin.x,
                           0,
                           from_frame.size.w,
                           from_frame.size.h);

    PropertyAnimation *down_animation = property_animation_create_layer_frame(s_base_layer, &from_frame, &to_frame);
    animation_set_handlers(property_animation_get_animation(down_animation),
                           (AnimationHandlers){.started = NULL,
                                               .stopped = s_down_animation_stopped_handler},
                           (void*)down_animation);
    animation_set_delay(property_animation_get_animation(down_animation), DELAY_UP_TO_DOWN);
    animation_set_curve(property_animation_get_animation(down_animation), AnimationCurveEaseOut);
    animation_schedule(property_animation_get_animation(down_animation));
}

static void s_display_time(void) {
    if (is_display_time == false) {
        is_display_time = true;
        
        GRect from_frame = layer_get_frame(s_base_layer);
        GRect to_frame = GRect(from_frame.origin.x,
                               layer_get_bounds(time_layer_get_layer(s_time_layer)).size.h * -1,
                               from_frame.size.w,
                               from_frame.size.h);
    
        time_layer_set_hidden(s_time_layer, false);
        PropertyAnimation *animation = property_animation_create_layer_frame(s_base_layer, &from_frame, &to_frame);
        animation_set_handlers(property_animation_get_animation(animation),
                               (AnimationHandlers){.started = NULL,
                                                   .stopped = s_up_animation_stopped_handler},
                               (void*)animation);
        animation_set_curve(property_animation_get_animation(animation), AnimationCurveEaseIn);
        animation_schedule(property_animation_get_animation(animation));
    }
}

static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void s_up_click_handler(ClickRecognizerRef recognizer, void *context) {
    s_display_time();
}

static void s_down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void s_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, s_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, s_down_click_handler);
}

static void s_accel_tap_handler(AccelAxisType axis, int32_t direction) {
    s_display_time();
}

static void s_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    // create time-layer
    s_time_layer = time_layer_create(window_bounds);
    layer_add_child(window_layer, time_layer_get_layer(s_time_layer));
    time_layer_set_hidden(s_time_layer, true);
    
    // create base-layer
    s_base_layer = layer_create(window_bounds);
    layer_set_update_proc(s_base_layer, s_base_layer_update_proc);
    layer_add_child(window_layer, s_base_layer);
    
    // create bitmap
    s_bmp[GREEN] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_GREEN);
    s_bmp[RED] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIGNAL_RED);

    // create bitmap-layer
    s_bmp_layer = bitmap_layer_create(window_bounds);
    bitmap_layer_set_background_color(s_bmp_layer, GColorBlack);
    bitmap_layer_set_bitmap(s_bmp_layer, s_bmp[GREEN]);
    layer_add_child(s_base_layer, bitmap_layer_get_layer(s_bmp_layer));

    // create time-layer
    s_timebar_layer[0] = timebar_layer_create(GPoint(5, 26));
    layer_add_child(s_base_layer, timebar_layer_get_layer(s_timebar_layer[0]));

    s_timebar_layer[1] = timebar_layer_create(GPoint(124, 26));
    layer_add_child(s_base_layer, timebar_layer_get_layer(s_timebar_layer[1]));
    
    // start tick-timer
    s_update(GREEN, false);
    tick_timer_service_subscribe(SECOND_UNIT, s_tick_handler);
    is_tick_timer_subscribe = true;
    
    // start tap-service
    accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
    accel_tap_service_subscribe(s_accel_tap_handler);
}

static void s_window_unload(Window *window) {
    accel_tap_service_unsubscribe();
    
    tick_timer_service_unsubscribe();
    
    timebar_layer_destroy(s_timebar_layer[1]);
    timebar_layer_destroy(s_timebar_layer[0]);
    
    bitmap_layer_destroy(s_bmp_layer);
    
    gbitmap_destroy(s_bmp[GREEN]);
    gbitmap_destroy(s_bmp[RED]);
    
    layer_destroy(s_base_layer);
    
    time_layer_destory(s_time_layer);
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