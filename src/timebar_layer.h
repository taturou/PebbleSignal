#pragma once
#include <pebble.h>

struct TimebarLayer;
typedef struct TimebarLayer TimebarLayer;

TimebarLayer *timebar_layer_create(GPoint origin);
void timebar_layer_destroy(TimebarLayer *timebar_layer);
Layer *timebar_layer_get_layer(TimebarLayer *timebar_layer);
void timebar_layer_set_hidden(TimebarLayer *timebar_layer, bool hidden);
void timebar_layer_set_bar_height(TimebarLayer *timebar_layer, uint16_t height);
void timebar_layer_set_bar_color(TimebarLayer *timebar_layer, GColor8 color_fg, GColor8 color_bg);
void timebar_layer_set_value(TimebarLayer *timebar_layer, int value);