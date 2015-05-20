#pragma once
#include <pebble.h>

enum Configurations {
    C_ConfigVersion = 0,  /* uint32 */
    C_VibesEachHour,      /* int8 -> bool */
    C_TimebarPattern,     /* int32 -> TimebarPattern  */
    C_TimePattern,        /* int32 -> TimePattern  */
};
typedef enum Configurations Configurations;

enum TimebarPattern {
    TBP_Signal = 0,
    TBP_HourAndMin,
    TBP_None,
};
typedef enum TimebarPattern TimebarPattern;

enum TimePattern {
    TP_MDD_h_mm = 0,
    TP_h_mm_ss,
};
typedef enum TimePattern TimePattern;

typedef void (*ConfigurationUpdatedHandler)(void *context);

struct ConfigurationHandlers {
    ConfigurationUpdatedHandler updated;
};
typedef struct ConfigurationHandlers ConfigurationHandlers;

void configuration_load(void);
void configuration_set_handlers(ConfigurationHandlers callback, void *context);
void *configuration_get(Configurations config);