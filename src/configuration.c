#include <pebble.h>
#include "configuration.h"

#define STORAGE_KEY_CONFIG_VERSION  (1)
#define STORAGE_KEY_V1_DICTIONARY   (2)
    
#define CURRENT_CONFIG_VERSION      (1)
#define NUM_KEY_CONFIG              (3)

#define KEY_CONF_CONFIG_VERSION     (0)
#define KEY_CONF_VIBES_EACH_HOUR    (1)
#define KEY_CONF_TIMEBAR_PATTERN    (2)
#define KEY_CONF_TIME_PATTERN       (3)

#define DEFAULT_VALUE_CONF_CONFIG_VERSION     (CURRENT_CONFIG_VERSION)
#define DEFAULT_VALUE_CONF_VIBES_EACH_HOUR    (true)
#define DEFAULT_VALUE_CONF_TIMEBAR_PATTERN    (TBP_Signal)
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
                                         sizeof(int8_t),   /* vibes_each_hour */
                                         sizeof(int32_t),  /* timebar_pattern */
                                         sizeof(int32_t)); /* time_pattern */
}

static ConfigData *s_get_config_data(void) {
    static ConfigData data;
    return &data;
}

static void s_init_config_data(void) {
    ConfigData *data = s_get_config_data();
    data->vibes_each_hour = DEFAULT_VALUE_CONF_VIBES_EACH_HOUR;
    data->timebar_pattern = DEFAULT_VALUE_CONF_TIMEBAR_PATTERN;
    data->time_pattern = DEFAULT_VALUE_CONF_TIME_PATTERN;
}

static ConfigHandlersData *s_get_config_handlers_data(void) {
    static ConfigHandlersData data;
    return &data;
}

static void s_init_config_handlers_data(void) {
    ConfigHandlersData *data = s_get_config_handlers_data();
    data->callback.updated = NULL;
    data->context = NULL;
}

static void s_persist_clear(void) {
    (void)persist_delete(STORAGE_KEY_CONFIG_VERSION);
    (void)persist_delete(STORAGE_KEY_V1_DICTIONARY);
    s_init_config_data();
}

static void s_persist_read_v1(void) {
    bool success = false;

    if (persist_exists(STORAGE_KEY_V1_DICTIONARY) == true) {
        const size_t buffer_size = s_get_dict_buffer_size();
        uint8_t buffer[buffer_size];
        
        if (persist_read_data(STORAGE_KEY_V1_DICTIONARY, buffer, buffer_size) == (int)buffer_size) {
            s_init_config_data();
            ConfigData *data = s_get_config_data();

            DictionaryIterator iter;
            Tuple *tuple = dict_read_begin_from_buffer(&iter, buffer, buffer_size);
            while (tuple != NULL) {
                switch (tuple->key) {
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
                tuple = dict_read_next(&iter);
            }
            success = true;
        }
    }
    if (success == false) {
        s_persist_clear();
    }
}

static void s_persist_migrate(void) {
    uint32_t version = persist_read_int(STORAGE_KEY_CONFIG_VERSION); // defaults to 0 if key is missing.

    switch(version) {
    case 0:
        s_persist_clear();
        break;
    case 1:
        s_persist_read_v1();
        break;
     default:
        s_persist_clear();
        break;
    }
}
    
void configuration_load(void) {
    s_init_config_data();
    s_init_config_handlers_data();
    s_persist_migrate();
    
    ConfigData *data = s_get_config_data();
}

void configuration_set_handlers(ConfigurationHandlers callback, void *context) {
    ConfigHandlersData *data = s_get_config_handlers_data();
    data->callback = callback;
    data->context = context;
}

void *configuration_get(Configurations config) {
    ConfigData *data = s_get_config_data();
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