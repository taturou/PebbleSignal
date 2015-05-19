#pragma once
#include <pebble.h>

struct SignalLayer;
typedef struct SignalLayer SignalLayer;

SignalLayer *signal_layer_create(GRect window_bounds);
void signal_layer_destroy(SignalLayer *signal_layer);
Layer *signal_layer_get_layer(SignalLayer *signal_layer);
void signal_layer_set_glance_height(SignalLayer *signal_layer, uint16_t height);
void signal_layer_tick_handler(SignalLayer *signal_layer, struct tm *tick_time, TimeUnits units_changed);
void siangl_layer_display_time(SignalLayer *signal_layer);