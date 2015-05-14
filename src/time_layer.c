#include <pebble.h>
#include "time_layer.h"

struct TimeLayer {
    Layer *layer;
};

#define L_MARGIN_TOP      (2)
#define L_MARGIN_BOTTOM   (2)
#define L_MARGIN_LEFT     (2)
#define L_MARGIN_RIGHT    (2)
#define L_MARGIN_NEXT     (40)
#define L_RADIUS          (25)
#define L_CENTER_X_1      (L_RADIUS + L_MARGIN_LEFT)
#define L_CENTER_Y_1      (L_RADIUS + L_MARGIN_TOP)
#define L_CENTER_X_2      (L_CENTER_X_1 + (L_RADIUS * 2 + L_MARGIN_NEXT))
#define L_CENTER_Y_2      (L_CENTER_Y_1)
#define L_RECT_ORIGIN_X   (L_CENTER_X_1)
#define L_RECT_ORIGIN_Y   (L_MARGIN_TOP)
#define L_RECT_SIZE_W     (L_RADIUS * 2 + L_MARGIN_NEXT)
#define L_RECT_SIZE_H     (L_RADIUS * 2 + 1)

#define S_MARGIN_TOP      (7)
#define S_MARGIN_BOTTOM   (7)
#define S_MARGIN_LEFT     (7)
#define S_MARGIN_RIGHT    (7)
#define S_MARGIN_NEXT     (5)
#define S_RADIUS          (20)
#define S_CENTER_X_1      (S_RADIUS + S_MARGIN_LEFT)
#define S_CENTER_Y_1      (S_RADIUS + S_MARGIN_TOP)
#define S_CENTER_X_2      (S_CENTER_X_1 + (S_RADIUS * 2 + S_MARGIN_NEXT))
#define S_CENTER_Y_2      (S_CENTER_Y_1)
#define S_CENTER_X_3      (S_CENTER_X_2 + (S_RADIUS * 2 + S_MARGIN_NEXT))
#define S_CENTER_Y_3      (S_CENTER_Y_1)

static GRect s_grect_circle_to_rect(GPoint p, uint16_t radius) {
    GRect rect;
    
    rect.origin.x = p.x - radius;
    rect.origin.y = p.y - radius;
    rect.size.w = radius * 2;
    rect.size.h = rect.size.w;
    
    return rect;
}

static void s_layer_update_proc(struct Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_antialiased(ctx, true);

    // time
    time_t now_time = time(NULL);
    struct tm *now_tm = localtime(&now_time);

    // text
    char str[32];
    GPoint p;
    graphics_context_set_text_color(ctx, GColorDarkGray);

    // background
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // case of signal
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_circle(ctx, GPoint(L_CENTER_X_1, L_CENTER_Y_1), L_RADIUS);
    graphics_fill_circle(ctx, GPoint(L_CENTER_X_2, L_CENTER_Y_2), L_RADIUS);
    graphics_fill_rect(ctx, GRect(L_RECT_ORIGIN_X, L_RECT_ORIGIN_Y, L_RECT_SIZE_W, L_RECT_SIZE_H), 0, GCornerNone);

    // signal (green)
    graphics_context_set_fill_color(ctx, GColorJaegerGreen);
    p = GPoint(S_CENTER_X_1, S_CENTER_Y_1);
    graphics_fill_circle(ctx, p, S_RADIUS);
    snprintf(str, 31, "%d", now_tm->tm_hour);
    graphics_draw_text(ctx,
                       str,
                       fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                       s_grect_circle_to_rect(p, S_RADIUS),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);

    // signal (yellow)
    graphics_context_set_fill_color(ctx, GColorChromeYellow);
    p = GPoint(S_CENTER_X_2, S_CENTER_Y_2);
    graphics_fill_circle(ctx, p, S_RADIUS);
    snprintf(str, 31, "%02d", now_tm->tm_min);
    graphics_draw_text(ctx,
                       str,
                       fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                       s_grect_circle_to_rect(p, S_RADIUS),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);

    // signal (red)
    graphics_context_set_fill_color(ctx, GColorRed);
    p = GPoint(S_CENTER_X_3, S_CENTER_Y_3);
    graphics_fill_circle(ctx, p, S_RADIUS);
    snprintf(str, 31, "%02d", now_tm->tm_sec);
    graphics_draw_text(ctx,
                       str,
                       fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                       s_grect_circle_to_rect(p, S_RADIUS),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
}
    
TimeLayer *time_layer_create(GRect window_bounds) {
    TimeLayer *time_layer = NULL;
    
    GRect frame;
    frame.size.w = window_bounds.size.w;
    frame.size.h = (L_MARGIN_TOP + L_MARGIN_BOTTOM + L_RADIUS * 2);
    frame.origin.x = 0;
    frame.origin.y = window_bounds.size.h - frame.size.h;
    
    Layer *layer = layer_create_with_data(frame, sizeof(TimeLayer));
    if (layer != NULL) {
        time_layer = (TimeLayer*)layer_get_data(layer);
        time_layer->layer = layer;
        layer_set_update_proc(time_layer->layer, s_layer_update_proc);
    }
    return time_layer;
}

void time_layer_destory(TimeLayer *time_layer) {
    layer_destroy(time_layer->layer);
}

Layer *time_layer_get_layer(TimeLayer *time_layer) {
    return time_layer->layer;
}

void time_layer_set_hidden(TimeLayer *time_layer, bool hidden) {
    layer_set_hidden(time_layer->layer, hidden);
}