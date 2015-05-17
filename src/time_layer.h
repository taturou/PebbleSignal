#pragma once
#include <pebble.h>

struct TimeLayer;
typedef struct TimeLayer TimeLayer;
    
TimeLayer *time_layer_create(GRect window_bounds);
void time_layer_destroy(TimeLayer *time_layer);
Layer *time_layer_get_layer(TimeLayer *time_layer);
void time_layer_set_hidden(TimeLayer *time_layer, bool hidden);