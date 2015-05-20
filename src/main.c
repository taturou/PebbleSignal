#include <pebble.h>
#include "configuration.h"
#include "resource.h"
#include "time_layer.h"
#include "signal_layer.h"

static Window *s_window;
static TimeLayer *s_time_layer;
static SignalLayer *s_signal_layer;

static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void s_up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void s_down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void s_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, s_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, s_down_click_handler);
}

static void s_config_updated_handler(void *context) {
    (void)context;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_config_updated_handler");
    time_layer_config_updated(s_time_layer);
    signal_layer_config_updated(s_signal_layer);
}

static void s_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    signal_layer_tick_handler(s_signal_layer, tick_time, units_changed);
}

static void s_accel_tap_handler(AccelAxisType axis, int32_t direction) {
    siangl_layer_display_time(s_signal_layer);
}

static void s_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    configuration_load();
    resource_alloc();
    
    // create time-layer
    s_time_layer = time_layer_create(window_bounds);
    layer_add_child(window_layer, time_layer_get_layer(s_time_layer));
    
    // create signal-layer
    s_signal_layer = signal_layer_create(window_bounds);
    signal_layer_set_glance_height(s_signal_layer, layer_get_bounds(time_layer_get_layer(s_time_layer)).size.h);
    layer_add_child(window_layer, signal_layer_get_layer(s_signal_layer));
    
    // start tick-timer
    tick_timer_service_subscribe(SECOND_UNIT, s_tick_handler);
    
    // start tap-service
    accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
    accel_tap_service_subscribe(s_accel_tap_handler);

    // update configurations
    configuration_set_handlers(
        (ConfigurationHandlers){
            .updated = s_config_updated_handler
        },
        NULL);
    s_config_updated_handler(NULL);
    
    // display time
    siangl_layer_display_time(s_signal_layer);
}

static void s_window_unload(Window *window) {
    accel_tap_service_unsubscribe();
    tick_timer_service_unsubscribe();

    signal_layer_destroy(s_signal_layer);
    time_layer_destroy(s_time_layer);
    
    resource_free();
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