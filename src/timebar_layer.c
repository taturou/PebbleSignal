#include <pebble.h>
#include "timebar_layer.h"

struct TimebarLayer {
    Layer *layer;
    int value;
    uint16_t bar_height;
    GColor8 bar_color_fg;
    GColor8 bar_color_bg;
};

#define WIDTH    (15)
#define HEIGHT   (120)

static void s_layer_update_proc(struct Layer *layer, GContext *ctx) {
    TimebarLayer *timebar_layer = (TimebarLayer*)layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
    int max = HEIGHT / (timebar_layer->bar_height + 1);
    
    graphics_context_set_stroke_color(ctx, timebar_layer->bar_color_bg);
    for (int i = 0; i < timebar_layer->value; i++) {
        uint16_t y = i * (timebar_layer->bar_height + 1);
        graphics_draw_rect(ctx, GRect(0, y, WIDTH, timebar_layer->bar_height));
    }
    graphics_context_set_stroke_color(ctx, timebar_layer->bar_color_fg);
    for (int i = timebar_layer->value; i < max; i++) {
        uint16_t y = i * (timebar_layer->bar_height + 1);
        graphics_draw_rect(ctx, GRect(0, y, WIDTH, timebar_layer->bar_height));
    }
}
    
TimebarLayer *timebar_layer_create(GPoint origin) {
    TimebarLayer *timebar_layer = NULL;
    
    Layer *layer = layer_create_with_data((GRect){.origin = origin, .size={.w=WIDTH, .h=HEIGHT}}, sizeof(TimebarLayer));
    if (layer != NULL) {
        timebar_layer = (TimebarLayer*)layer_get_data(layer);
        timebar_layer->layer = layer;
        timebar_layer->value = 0;
        timebar_layer->bar_height = 1;
        timebar_layer->bar_color_fg = GColorBlack;
        timebar_layer->bar_color_bg = GColorBlack;
        layer_set_update_proc(timebar_layer->layer, s_layer_update_proc);
    }
    return timebar_layer;
}

void timebar_layer_destroy(TimebarLayer *timebar_layer) {
    layer_destroy(timebar_layer->layer);
}

Layer *timebar_layer_get_layer(TimebarLayer *timebar_layer) {
    return timebar_layer->layer;
}

void timebar_layer_set_hidden(TimebarLayer *timebar_layer, bool hidden) {
    layer_set_hidden(timebar_layer->layer, hidden);
}

void timebar_layer_set_bar_height(TimebarLayer *timebar_layer, uint16_t height) {
    if (timebar_layer->bar_height != height) {
        timebar_layer->bar_height = height;
        layer_mark_dirty(timebar_layer->layer);
    }
}

void timebar_layer_set_bar_color(TimebarLayer *timebar_layer, GColor8 color_fg, GColor8 color_bg) {
    if (gcolor_equal(timebar_layer->bar_color_fg, color_fg) == false) {
        timebar_layer->bar_color_fg = color_fg;
        layer_mark_dirty(timebar_layer->layer);
    }
    if (gcolor_equal(timebar_layer->bar_color_bg, color_bg) == false) {
        timebar_layer->bar_color_bg = color_bg;
        layer_mark_dirty(timebar_layer->layer);
    }
}

void timebar_layer_set_value(TimebarLayer *timebar_layer, int value) {
    if (timebar_layer->value != value) {
        timebar_layer->value = value;
        layer_mark_dirty(timebar_layer->layer);
    }
}