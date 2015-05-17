#include <pebble.h>
#include "signal_layer.h"
#include "resource.h"
#include "timebar_layer.h"

/* configurations */
#define DISPLAY_TIMEBAR_HOUR_AND_MIN (1)
#define DELAY_UP_TO_DOWN             (4000)

/* code */
#define NUM_TIMEBAR (2)

#define ORIGIN_X_1  (5)
#define ORIGIN_Y_1  (26)
#define ORIGIN_X_2  (124)
#define ORIGIN_Y_2  (26)

struct SignalLayer {
    Layer *layer;
    BitmapLayer *bitmap_layer;
    TimebarLayer *timebar_layer[NUM_TIMEBAR];
    bool display_time;
};

static void s_layer_update_proc(struct Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void s_down_animation_stopped_handler(Animation *hide_animation, bool finished, void *context) {
    SignalLayer *signal_layer = (SignalLayer*)context;

    animation_destroy(hide_animation);
    signal_layer->display_time = false;
}

static void s_up_animation_stopped_handler(Animation *show_animation, bool finished, void *context) {
    SignalLayer *signal_layer = (SignalLayer*)context;
    Layer *layer = signal_layer->layer;

    // destroy
    animation_destroy(show_animation);

    // start animation to hide
    GRect from_frame = layer_get_frame(layer);
    GRect to_frame = GRect(from_frame.origin.x,
                           0,
                           from_frame.size.w,
                           from_frame.size.h);

    PropertyAnimation *hide_animation = property_animation_create_layer_frame(layer, &from_frame, &to_frame);
    animation_set_handlers(property_animation_get_animation(hide_animation),
                           (AnimationHandlers){.started = NULL,
                                               .stopped = s_down_animation_stopped_handler},
                           (void*)context);
    animation_set_delay(property_animation_get_animation(hide_animation), DELAY_UP_TO_DOWN);
    animation_set_curve(property_animation_get_animation(hide_animation), AnimationCurveEaseOut);
    animation_schedule(property_animation_get_animation(hide_animation));
}

SignalLayer *signal_layer_create(GRect window_bounds) {
    SignalLayer *signal_layer = NULL;
    
    Layer *layer = layer_create_with_data(window_bounds, sizeof(SignalLayer));
    if (layer != NULL) {
        // setup layer
        layer_set_update_proc(layer, s_layer_update_proc);
    
        // create bitmap-layer
        BitmapLayer *bitmap_layer = bitmap_layer_create(window_bounds);
        bitmap_layer_set_background_color(bitmap_layer, GColorBlack);
        bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, On));
        layer_add_child(layer, bitmap_layer_get_layer(bitmap_layer));
    
        // create time-layer
        TimebarLayer *timebar_layer[NUM_TIMEBAR];
        GPoint timebar_origin[NUM_TIMEBAR];
        timebar_origin[0].x = ORIGIN_X_1;
        timebar_origin[0].y = ORIGIN_Y_1;
        timebar_origin[1].x = ORIGIN_X_2;
        timebar_origin[1].y = ORIGIN_Y_2;
        for (int i = 0; i < NUM_TIMEBAR; i++) {
            timebar_layer[i] = timebar_layer_create(timebar_origin[i]);
            layer_add_child(layer, timebar_layer_get_layer(timebar_layer[i]));            
        }

        // set members
        signal_layer = (SignalLayer*)layer_get_data(layer);
        signal_layer->layer = layer;
        signal_layer->bitmap_layer = bitmap_layer;
        signal_layer->display_time = false;
        for (int i = 0; i < NUM_TIMEBAR; i++) {
            signal_layer->timebar_layer[i] = timebar_layer[i];
        }        
    }
    return signal_layer;
}

void signal_layer_destroy(SignalLayer *signal_layer) {
    for (int i = 0; i < NUM_TIMEBAR; i++) {
        timebar_layer_destroy(signal_layer->timebar_layer[i]);
    }
    
    bitmap_layer_destroy(signal_layer->bitmap_layer);
    
    layer_destroy(signal_layer->layer);
}

Layer *signal_layer_get_layer(SignalLayer *signal_layer) {
    return signal_layer->layer;    
}

void signal_layer_tick_handler(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed) {
    BitmapLayer *bitmap_layer = signal_layer->bitmap_layer;
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    if ((tick_time->tm_min == 0) && (tick_time->tm_sec == 0)) {
        vibes_short_pulse();
    }
    
#if DISPLAY_TIMEBAR_HOUR_AND_MIN
    if ((tick_time->tm_min % 2) == 0) {
        if (tick_time->tm_sec < 59) {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Red, On));
        } else {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Red, Off));
        }
        timebar_layer_set_signal(timebar_layer[0], Red);
        timebar_layer_set_signal(timebar_layer[1], Red);
        timebar_layer_set_bar_height(timebar_layer[0], 1);
        timebar_layer_set_bar_height(timebar_layer[1], 1);
    } else {
        if (tick_time->tm_sec < 40) {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, On));

        } else {
            if ((tick_time->tm_sec % 2) == 0) {
                bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, On));
            } else {
                bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, Off));
            }
        }
        timebar_layer_set_signal(timebar_layer[0], Green);
        timebar_layer_set_signal(timebar_layer[1], Green);
        timebar_layer_set_bar_height(timebar_layer[0], 1);
        timebar_layer_set_bar_height(timebar_layer[1], 2);
    }
    timebar_layer_set_value(timebar_layer[0], tick_time->tm_min);
    timebar_layer_set_value(timebar_layer[1], tick_time->tm_sec);
#else /* ! DISPLAY_TIMEBAR_HOUR_AND_MIN */
    if ((tick_time->tm_min % 2) == 0) {
        if (tick_time->tm_sec < 59) {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Red, On));
        } else {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Red, Off));
        }
        timebar_layer_set_signal(timebar_layer[0], Red);
        timebar_layer_set_signal(timebar_layer[1], Red);
        timebar_layer_set_bar_height(timebar_layer[0], 5);
        timebar_layer_set_bar_height(timebar_layer[1], 5);
    } else {
        if (tick_time->tm_sec < 45) {
            bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, On));
        } else {
            if ((tick_time->tm_sec % 2) == 0) {
                bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, On));
            } else {
                bitmap_layer_set_bitmap(bitmap_layer, resource_get_bitmap(Green, Off));
            }
        }
        timebar_layer_set_signal(timebar_layer[0], Green);
        timebar_layer_set_signal(timebar_layer[1], Green);
        timebar_layer_set_bar_height(timebar_layer[0], 7);
        timebar_layer_set_bar_height(timebar_layer[1], 7);
    }
    timebar_layer_set_value(timebar_layer[0], tick_time->tm_sec / 3);
    timebar_layer_set_value(timebar_layer[1], tick_time->tm_sec / 3);
#endif /* ! DISPLAY_TIMEBAR_HOUR_AND_MIN */
}

void siangl_layer_display_time(SignalLayer *signal_layer, uint16_t size_h) {
    Layer *layer = signal_layer->layer;

    if (signal_layer->display_time == false) {
        signal_layer->display_time = true;

        GRect from_frame = layer_get_frame(layer);
        GRect to_frame = GRect(from_frame.origin.x,
                               size_h * -1,
                               from_frame.size.w,
                               from_frame.size.h);
    
        PropertyAnimation *show_animation = property_animation_create_layer_frame(layer, &from_frame, &to_frame);
        animation_set_handlers(property_animation_get_animation(show_animation),
                               (AnimationHandlers){.started = NULL,
                                                   .stopped = s_up_animation_stopped_handler},
                               (void*)signal_layer);
        animation_set_curve(property_animation_get_animation(show_animation), AnimationCurveEaseIn);
        animation_schedule(property_animation_get_animation(show_animation));
    }
}