#include <pebble.h>
#include "configuration.h"

#define STORAGE_KEY_CONFIG_VERSION  (1)
#define STORAGE_KEY_V1_DICTIONARY   (2)
    
#define CURRENT_CONFIG_VERSION      (1)
#define NUM_KEY_CONFIG              (3)

#define KEY_CONFIG_VERSION          (0)
#define KEY_NOTIFY_REQ_CONF         (1)
#define KEY_CONF_VIBES_EACH_HOUR    (2)
#define KEY_CONF_TIMEBAR_PATTERN    (3)
#define KEY_CONF_TIME_PATTERN       (4)

#define DEFAULT_VALUE_CONF_CONFIG_VERSION     (CURRENT_CONFIG_VERSION)
#define DEFAULT_VALUE_CONF_VIBES_EACH_HOUR    (true)
#define DEFAULT_VALUE_CONF_TIMEBAR_PATTERN    (TBP_None)
#define DEFAULT_VALUE_CONF_TIME_PATTERN       (TP_MDD_h_mm)

struct ConfigData {
    bool vibes_each_hour;
    TimebarPattern timebar_pattern;
    TimePattern time_pattern;
};
typedef struct ConfigData ConfigData;

struct ConfigHandlersData {
    ConfigurationHandlers callback;
    void *context;
};
typedef struct ConfigHandlersData ConfigHandlersData;

static size_t s_get_dict_buffer_size(void) {
    return (size_t)dict_calc_buffer_size(NUM_KEY_CONFIG,
                                         sizeof(int8_t),   // vibes_each_hour
                                         sizeof(int32_t),  // timebar_pattern
                                         sizeof(int32_t)); // time_pattern
}

static ConfigData *s_config_get_data(void) {
    static ConfigData data;
    return &data;
}

static void s_app_message_outbox_send(void);

static void s_config_init_data(void) {
    ConfigData *data = s_config_get_data();
    data->vibes_each_hour = DEFAULT_VALUE_CONF_VIBES_EACH_HOUR;
    data->timebar_pattern = DEFAULT_VALUE_CONF_TIMEBAR_PATTERN;
    data->time_pattern = DEFAULT_VALUE_CONF_TIME_PATTERN;
}

static ConfigHandlersData *s_config_get_handlers(void) {
    static ConfigHandlersData data;
    return &data;
}

static void s_config_init_handlers(void) {
    ConfigHandlersData *data = s_config_get_handlers();
    data->callback.updated = NULL;
    data->context = NULL;
}

static void s_config_read_from_dictionary(DictionaryIterator *iter) {
    ConfigData *data = s_config_get_data();

    Tuple *tuple = dict_read_first(iter);
    while (tuple != NULL) {
        switch (tuple->key) {
        case KEY_NOTIFY_REQ_CONF:
            s_app_message_outbox_send();
            break;
        case KEY_CONF_VIBES_EACH_HOUR:
            data->vibes_each_hour = (bool)tuple->value->int8;
            break;
        case KEY_CONF_TIMEBAR_PATTERN:
            data->timebar_pattern = (TimebarPattern)tuple->value->int32;
            break;
        case KEY_CONF_TIME_PATTERN:
            data->time_pattern = (TimePattern)tuple->value->int32;
            break;
        default:
            /* do nothing */
            break;
        }
        tuple = dict_read_next(iter);
    }
}

static void s_config_write_to_buffer(uint8_t *buffer, size_t buffer_size) {
    ConfigData *data = s_config_get_data();

    DictionaryIterator iter;
    dict_write_begin(&iter, buffer, buffer_size);
    dict_write_int8(&iter, KEY_CONF_VIBES_EACH_HOUR, data->vibes_each_hour);
    dict_write_int32(&iter, KEY_CONF_TIMEBAR_PATTERN, data->timebar_pattern);
    dict_write_int32(&iter, KEY_CONF_TIME_PATTERN, data->time_pattern);
    dict_write_end(&iter);
}

static void s_persist_clear(void) {
    (void)persist_delete(STORAGE_KEY_CONFIG_VERSION);
    (void)persist_delete(STORAGE_KEY_V1_DICTIONARY);
    s_config_init_data();
}

static void s_persist_read_v1(void) {
    bool success = false;

    if (persist_exists(STORAGE_KEY_V1_DICTIONARY) == true) {
        const size_t buffer_size = s_get_dict_buffer_size();
        uint8_t buffer[buffer_size];
        
        if (persist_read_data(STORAGE_KEY_V1_DICTIONARY, buffer, buffer_size) == (int)buffer_size) {
            s_config_init_data();

            DictionaryIterator iter;
            (void)dict_read_begin_from_buffer(&iter, buffer, buffer_size);
            s_config_read_from_dictionary(&iter);
            success = true;
        }
    }
    if (success == false) {
        s_persist_clear();
    }
}

static void s_persist_write_v1(void) {
    const size_t buffer_size = s_get_dict_buffer_size();
    uint8_t buffer[buffer_size];

    s_config_write_to_buffer(buffer, buffer_size);

    (void)persist_write_int(STORAGE_KEY_CONFIG_VERSION, CURRENT_CONFIG_VERSION);
    (void)persist_write_data(STORAGE_KEY_V1_DICTIONARY, buffer, buffer_size);
}

static void s_persist_migrate(void) {
    uint32_t version = persist_read_int(STORAGE_KEY_CONFIG_VERSION); // defaults to 0 if key is missing.

    switch(version) {
    case 1:
        s_persist_read_v1();
        break;
    case 0:
        /* fall down */
    default:
        s_persist_clear();
        break;
    }
}

static void s_app_message_inbox_received_callback(DictionaryIterator *iter, void *context) {
    s_config_read_from_dictionary(iter);
    s_persist_write_v1();

    ConfigHandlersData *data = s_config_get_handlers();
    data->callback.updated(data->context);
}

static void s_app_message_inbox_dropped_callback(AppMessageResult reason, void *context) {
    /* do nothing */
}

static void s_app_message_outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    /* do nothing */
}

static void s_app_message_outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    /* do nothing */
}

static void s_app_message_outbox_send(void) {
    ConfigData *data = s_config_get_data();

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_int8(iter, KEY_CONF_VIBES_EACH_HOUR, (int8_t)data->vibes_each_hour);
    dict_write_int32(iter, KEY_CONF_TIMEBAR_PATTERN, (int32_t)data->timebar_pattern);
    dict_write_int32(iter, KEY_CONF_TIME_PATTERN, (int32_t)data->time_pattern);
    dict_write_end (iter);
    app_message_outbox_send();
}

static void s_app_message_set_callback(void) {
    // Register callbacks
    app_message_register_inbox_received(s_app_message_inbox_received_callback);
    app_message_register_inbox_dropped(s_app_message_inbox_dropped_callback);
    app_message_register_outbox_failed(s_app_message_outbox_failed_callback);
    app_message_register_outbox_sent(s_app_message_outbox_sent_callback);

    // Open AppMessage
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void configuration_load(void) {
    // init data
    s_config_init_data();
    s_config_init_handlers();
    s_persist_migrate();

    // set app-message callback
    s_app_message_set_callback();
}

void configuration_set_handlers(ConfigurationHandlers callback, void *context) {
    ConfigHandlersData *data = s_config_get_handlers();
    data->callback = callback;
    data->context = context;
}

void *configuration_get(Configurations config) {
    ConfigData *data = s_config_get_data();
    void *ret = NULL;

    switch (config) {
    case C_ConfigVersion:
        ret = (void*)CURRENT_CONFIG_VERSION;
        break;
    case C_VibesEachHour:
        ret = (void*)data->vibes_each_hour;
        break;
    case C_TimebarPattern:
        ret = (void*)data->timebar_pattern;
        break;
    case C_TimePattern:
        ret = (void*)data->time_pattern;
        break;
    default:
        ret = NULL;
        break;
    }
    return ret;
}