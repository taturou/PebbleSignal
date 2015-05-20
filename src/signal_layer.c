#include <pebble.h>
#include "signal_layer.h"
#include "configuration.h"
#include "resource.h"
#include "timebar_layer.h"

#define NUM_TIMEBAR (2)

#define ORIGIN_X_1  (5)
#define ORIGIN_Y_1  (26)
#define ORIGIN_X_2  (124)
#define ORIGIN_Y_2  (26)

#define NUM_BITMAP_LAYER (2)

#define GLANCE_DELAY_UP_TO_DOWN      (4000)
#define GLANCE_DURATION              (250)
#define SIGNAL_DURATION              (150)

struct SignalLayer {
    Layer *layer;
    Layer *bitmap_base_layer;
    BitmapLayer *bitmap_layer[NUM_BITMAP_LAYER]; // 0:Green, 1:Red
    TimebarLayer *timebar_layer[NUM_TIMEBAR];
    uint16_t glance_height;
    Signal signal;
    TimebarPattern timebar_pattern;
    bool vibes_each_hour;
    bool glancing;
};

static void s_layer_update_proc(struct Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void s_glance_down_animation_stopped_handler(Animation *hide_animation, bool finished, void *context) {
    SignalLayer *signal_layer = (SignalLayer*)context;

    animation_destroy(hide_animation);
    signal_layer->glancing = false;
}

static void s_glance_up_animation_stopped_handler(Animation *show_animation, bool finished, void *context) {
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
                                               .stopped = s_glance_down_animation_stopped_handler},
                           (void*)context);
    animation_set_delay(property_animation_get_animation(hide_animation), GLANCE_DELAY_UP_TO_DOWN);
    animation_set_duration(property_animation_get_animation(hide_animation), GLANCE_DURATION);
    animation_set_curve(property_animation_get_animation(hide_animation), AnimationCurveEaseOut);
    animation_schedule(property_animation_get_animation(hide_animation));
}

static void s_glance(SignalLayer *signal_layer) {
    Layer *layer = signal_layer->layer;

    if (signal_layer->glancing == false) {
        signal_layer->glancing = true;

        GRect from_frame = layer_get_frame(layer);
        GRect to_frame = GRect(from_frame.origin.x,
                               signal_layer->glance_height * -1,
                               from_frame.size.w,
                               from_frame.size.h);
    
        PropertyAnimation *show_animation = property_animation_create_layer_frame(layer, &from_frame, &to_frame);
        animation_set_handlers(property_animation_get_animation(show_animation),
                               (AnimationHandlers){.started = NULL,
                                                   .stopped = s_glance_up_animation_stopped_handler},
                               (void*)signal_layer);
        animation_set_curve(property_animation_get_animation(show_animation), AnimationCurveEaseOut);
        animation_set_duration(property_animation_get_animation(show_animation), GLANCE_DURATION);
        animation_schedule(property_animation_get_animation(show_animation));
    }
}

static void s_set_onoff(SignalLayer *signal_layer, OnOff onoff) {
    bitmap_layer_set_bitmap(signal_layer->bitmap_layer[signal_layer->signal], resource_get_bitmap(signal_layer->signal, onoff));
}

static void s_signal_animation_stopped_handler(Animation *animation, bool finished, void *context) {
    SignalLayer *signal_layer = (SignalLayer*)context;
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    // destroy animation
    animation_destroy(animation);
    
    // set to Off
    s_set_onoff(signal_layer, On);
    timebar_layer_set_onoff(timebar_layer[0], On);
    timebar_layer_set_onoff(timebar_layer[1], On);
}

static void s_set_signal(SignalLayer *signal_layer, Signal signal) {
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    if (signal_layer->signal != signal) {
        signal_layer->signal = signal;

        // set to Off
        s_set_onoff(signal_layer, Off);
        timebar_layer_set_onoff(timebar_layer[0], Off);
        timebar_layer_set_onoff(timebar_layer[1], Off);

        // calc frame
        GRect frame = layer_get_bounds(signal_layer->bitmap_base_layer);
        if (signal == Green) {
            frame.origin.y = (frame.size.h / 2) * -1;
        } else {
            /* do nothing */
        }

        // animation
        PropertyAnimation *animation = property_animation_create_layer_frame(signal_layer->bitmap_base_layer, NULL, &frame);
        animation_set_handlers(property_animation_get_animation(animation),
                               (AnimationHandlers){.started = NULL,
                                                   .stopped = s_signal_animation_stopped_handler},
                               (void*)signal_layer);
        animation_set_duration(property_animation_get_animation(animation), SIGNAL_DURATION);
        animation_schedule(property_animation_get_animation(animation));
    }
}

static void s_tick_display_HourAndMin(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed) {
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    if ((tick_time->tm_min % 2) == 0) {
        s_set_signal(signal_layer, Red);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            s_set_onoff(signal_layer, On);
            timebar_layer_set_onoff(timebar_layer[0], On);
            timebar_layer_set_onoff(timebar_layer[1], On);
        }

        timebar_layer_set_signal(timebar_layer[0], Red);
        timebar_layer_set_signal(timebar_layer[1], Red);
        timebar_layer_set_bar_height(timebar_layer[0], 1);
        timebar_layer_set_bar_height(timebar_layer[1], 1);
    } else {
        s_set_signal(signal_layer, Green);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            if (tick_time->tm_sec < 40) {
                s_set_onoff(signal_layer, On);
            } else {
                if ((tick_time->tm_sec % 2) == 0) {
                    s_set_onoff(signal_layer, On);
                } else {
                    s_set_onoff(signal_layer, Off);
                }
            }
            timebar_layer_set_onoff(timebar_layer[0], On);
            timebar_layer_set_onoff(timebar_layer[1], On);
        } 
        
        timebar_layer_set_signal(timebar_layer[0], Green);
        timebar_layer_set_signal(timebar_layer[1], Green);
        timebar_layer_set_bar_height(timebar_layer[0], 1);
        timebar_layer_set_bar_height(timebar_layer[1], 2);
    }
    timebar_layer_set_value(timebar_layer[0], tick_time->tm_min);
    timebar_layer_set_value(timebar_layer[1], tick_time->tm_sec);
    timebar_layer_set_hidden(timebar_layer[0], false);
    timebar_layer_set_hidden(timebar_layer[1], false);
}

static void s_tick_display_Signal(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed) {
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    if ((tick_time->tm_min % 2) == 0) {
        s_set_signal(signal_layer, Red);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            s_set_onoff(signal_layer, On);
            timebar_layer_set_onoff(timebar_layer[0], On);
            timebar_layer_set_onoff(timebar_layer[1], On);
        }

        timebar_layer_set_signal(timebar_layer[0], Red);
        timebar_layer_set_signal(timebar_layer[1], Red);
        timebar_layer_set_bar_height(timebar_layer[0], 5);
        timebar_layer_set_bar_height(timebar_layer[1], 5);
    } else {
        s_set_signal(signal_layer, Green);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            if (tick_time->tm_sec < 45) {
                s_set_onoff(signal_layer, On);
            } else {
                if ((tick_time->tm_sec % 2) == 0) {
                    s_set_onoff(signal_layer, On);
                } else {
                    s_set_onoff(signal_layer, Off);
                }
            }
            timebar_layer_set_onoff(timebar_layer[0], On);
            timebar_layer_set_onoff(timebar_layer[1], On);
        }

        timebar_layer_set_signal(timebar_layer[0], Green);
        timebar_layer_set_signal(timebar_layer[1], Green);
        timebar_layer_set_bar_height(timebar_layer[0], 7);
        timebar_layer_set_bar_height(timebar_layer[1], 7);
    }
    timebar_layer_set_value(timebar_layer[0], tick_time->tm_sec / 3);
    timebar_layer_set_value(timebar_layer[1], tick_time->tm_sec / 3);
    timebar_layer_set_hidden(timebar_layer[0], false);
    timebar_layer_set_hidden(timebar_layer[1], false);
}

static void s_tick_display_None(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed) {
    TimebarLayer **timebar_layer = signal_layer->timebar_layer;

    if ((tick_time->tm_min % 2) == 0) {
        s_set_signal(signal_layer, Red);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            s_set_onoff(signal_layer, On);
        }
    } else {
        s_set_signal(signal_layer, Green);
        if (tick_time->tm_sec == 0) {
            /* do nothing for animation */
        } else {
            if (tick_time->tm_sec < 45) {
                s_set_onoff(signal_layer, On);
            } else {
                if ((tick_time->tm_sec % 2) == 0) {
                    s_set_onoff(signal_layer, On);
                } else {
                    s_set_onoff(signal_layer, Off);
                }
            }
        }
    }
    timebar_layer_set_hidden(timebar_layer[0], true);
    timebar_layer_set_hidden(timebar_layer[1], true);
}

SignalLayer *signal_layer_create(GRect window_bounds) {
    SignalLayer *signal_layer = NULL;
    
    Layer *layer = layer_create_with_data(window_bounds, sizeof(SignalLayer));
    if (layer != NULL) {
        // setup layer
        layer_set_update_proc(layer, s_layer_update_proc);
    
        // create bitmap-base-layer
        GRect bitmap_base_layer_bounds = GRect(window_bounds.origin.x,
                                               window_bounds.origin.y,
                                               window_bounds.size.w,
                                               window_bounds.size.h * 2);
        Layer *bitmap_base_layer = layer_create(bitmap_base_layer_bounds);
        layer_add_child(layer, bitmap_base_layer);

        // create bitmap-layer
        BitmapLayer *bitmap_layer[NUM_BITMAP_LAYER];

        GRect red_bitmap_layer_frame = window_bounds;
        bitmap_layer[Red] = bitmap_layer_create(red_bitmap_layer_frame);
        bitmap_layer_set_background_color(bitmap_layer[Red], GColorBlack);
        bitmap_layer_set_bitmap(bitmap_layer[Red], resource_get_bitmap(Red, On));
        layer_add_child(bitmap_base_layer, bitmap_layer_get_layer(bitmap_layer[Red]));
        
        GRect green_bitmap_layer_frame = GRect(window_bounds.origin.x,
                                               window_bounds.size.h,
                                               window_bounds.size.w,
                                               window_bounds.size.h);
        bitmap_layer[Green] = bitmap_layer_create(green_bitmap_layer_frame);
        bitmap_layer_set_background_color(bitmap_layer[Green], GColorBlack);
        bitmap_layer_set_bitmap(bitmap_layer[Green], resource_get_bitmap(Green, On));
        layer_add_child(bitmap_base_layer, bitmap_layer_get_layer(bitmap_layer[Green]));
    
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
        for (int i = 0; i < NUM_BITMAP_LAYER; i++) {
            signal_layer->bitmap_layer[i] = bitmap_layer[i];
        }
        signal_layer->bitmap_base_layer = bitmap_base_layer;
        for (int i = 0; i < NUM_TIMEBAR; i++) {
            signal_layer->timebar_layer[i] = timebar_layer[i];
        }        
        signal_layer->glance_height = window_bounds.size.h;
        signal_layer->signal = Red;
        signal_layer->timebar_pattern = TBP_Signal;
        signal_layer->vibes_each_hour = true;
        signal_layer->glancing = false;
        signal_layer_config_updated(signal_layer);
        
        // diaplay
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        signal_layer_tick_handler(signal_layer, tm, SECOND_UNIT);
    }
    return signal_layer;
}

void signal_layer_destroy(SignalLayer *signal_layer) {
    for (int i = 0; i < NUM_TIMEBAR; i++) {
        timebar_layer_destroy(signal_layer->timebar_layer[i]);
    }

    for (int i = 0; i < NUM_BITMAP_LAYER; i++) {
        bitmap_layer_destroy(signal_layer->bitmap_layer[i]);
    }
    layer_destroy(signal_layer->bitmap_base_layer);
    
    layer_destroy(signal_layer->layer);
}

Layer *signal_layer_get_layer(SignalLayer *signal_layer) {
    return signal_layer->layer;    
}

void signal_layer_set_glance_height(SignalLayer *signal_layer, uint16_t height) {
    signal_layer->glance_height = height;
}

void signal_layer_tick_handler(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed) {
    // vibes
    if (signal_layer->vibes_each_hour == true) {
        if ((tick_time->tm_min == 0) && (tick_time->tm_sec == 0)) {
            vibes_short_pulse();
        }
    }

    // display
    switch (signal_layer->timebar_pattern) {
    case TBP_HourAndMin:
        s_tick_display_HourAndMin(signal_layer, tick_time, units_changed);
        break;
    case TBP_None:
        s_tick_display_None(signal_layer, tick_time, units_changed);
        break;
    case TBP_Signal:
        /* fall down */
    default:
        s_tick_display_Signal(signal_layer, tick_time, units_changed);
        break;
    };
}

void siangl_layer_display_time(SignalLayer *signal_layer) {
    s_glance(signal_layer);
}

void signal_layer_config_updated(SignalLayer *signal_layer) {
    // vibes-each-hour
    signal_layer->vibes_each_hour = (bool)configuration_get(C_VibesEachHour);
    
    // timebar-pattern
    signal_layer->timebar_pattern = (TimebarPattern)configuration_get(C_TimebarPattern);
}