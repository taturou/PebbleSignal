#include <pebble.h>
#include "time_layer.h"

/* configurations */
#define DISPLAY_DATE          (1)
#define DISPLAY_CIRCLE_STROKE (0)

/* code */
struct TimeLayer {
    Layer *layer;
};

#define L_MARGIN_TOP      (2)
#define L_MARGIN_BOTTOM   (2)
#define L_RECT_ORIGIN_X   (2)
#define L_RECT_ORIGIN_Y   (2)
#define L_RECT_SIZE_W     (140)
#define L_RECT_SIZE_H     (50)
#define L_RADIUS          (25)

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

static GRect s_grect_change_size(GRect rect, GSize size) {
    int16_t dw = rect.size.w - size.w;
    int16_t dh = rect.size.h - size.h;

    rect.origin.x += dw / 2;
    rect.origin.y += dh / 2;
    rect.size = size;

    rect.origin.x += 1;            // magic number
    rect.origin.y -= size.h / 5;   // magic number

    return rect;
}

static void s_graphics_draw_text(GContext *ctx, const char *text, GFont const font, const GRect box) {
    GTextOverflowMode overflow_mode = GTextOverflowModeWordWrap;
    GTextAlignment alignment = GTextAlignmentCenter;
        
    GSize text_size = graphics_text_layout_get_content_size(text, font, box, overflow_mode, alignment);
    GRect text_box = s_grect_change_size(box, text_size);

    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, text, font, text_box, overflow_mode, alignment, NULL);
#if 0
    graphics_context_set_stroke_color(ctx, GColorPurple);
    graphics_draw_rect(ctx, box);
    graphics_draw_rect(ctx, text_box);
#endif
}

static void s_layer_update_proc(struct Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_antialiased(ctx, true);
#if DISPLAY_CIRCLE_STROKE
   graphics_context_set_stroke_width(ctx, 3);
#endif /* DISPLAY_CIRCLE_STROKE */

    // time
    time_t now_time = time(NULL);
    struct tm *now_tm = localtime(&now_time);

    // text
    char str[32];

    // background
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // case of signal
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(L_RECT_ORIGIN_X, L_RECT_ORIGIN_Y, L_RECT_SIZE_W, L_RECT_SIZE_H), L_RADIUS, GCornersAll);

    // signal (green)
    GPoint p1 = GPoint(S_CENTER_X_1, S_CENTER_Y_1);
    graphics_context_set_fill_color(ctx, GColorJaegerGreen);
    graphics_fill_circle(ctx, p1, S_RADIUS);
#if DISPLAY_CIRCLE_STROKE
    graphics_context_set_stroke_color(ctx, GColorMidnightGreen);
    graphics_draw_circle(ctx, p1, S_RADIUS);
#endif /* DISPLAY_CIRCLE_STROKE */

    // signal (yellow)
    GPoint p2 = GPoint(S_CENTER_X_2, S_CENTER_Y_2);
    graphics_context_set_fill_color(ctx, GColorChromeYellow);
    graphics_fill_circle(ctx, p2, S_RADIUS);
#if DISPLAY_CIRCLE_STROKE
    graphics_context_set_stroke_color(ctx, GColorWindsorTan);
    graphics_draw_circle(ctx, p2, S_RADIUS);
#endif /* DISPLAY_CIRCLE_STROKE */

    // signal (red)
    GPoint p3 = GPoint(S_CENTER_X_3, S_CENTER_Y_3);
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_circle(ctx, p3, S_RADIUS);
#if DISPLAY_CIRCLE_STROKE
    graphics_context_set_stroke_color(ctx, GColorBulgarianRose);
    graphics_draw_circle(ctx, p3, S_RADIUS);
#endif /* DISPLAY_CIRCLE_STROKE */

#if DISPLAY_DATE
    snprintf(str, 31, "%d/%d", now_tm->tm_mon+1, now_tm->tm_mday);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), s_grect_circle_to_rect(p1, S_RADIUS));
    
    snprintf(str, 31, "%d", now_tm->tm_hour);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), s_grect_circle_to_rect(p2, S_RADIUS));

    snprintf(str, 31, "%02d", now_tm->tm_min);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), s_grect_circle_to_rect(p3, S_RADIUS));
#else /* ! DISPLAY_DATE */
    snprintf(str, 31, "%d", now_tm->tm_hour);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), s_grect_circle_to_rect(p1, S_RADIUS));

    snprintf(str, 31, "%02d", now_tm->tm_min);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), s_grect_circle_to_rect(p2, S_RADIUS));

    snprintf(str, 31, "%02d", now_tm->tm_sec);
    s_graphics_draw_text(ctx, str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), s_grect_circle_to_rect(p3, S_RADIUS));
#endif /* ! DISPLAY_DATE */
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

void time_layer_destroy(TimeLayer *time_layer) {
    layer_destroy(time_layer->layer);
}

Layer *time_layer_get_layer(TimeLayer *time_layer) {
    return time_layer->layer;
}

void time_layer_set_hidden(TimeLayer *time_layer, bool hidden) {
    layer_set_hidden(time_layer->layer, hidden);
}