#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>
#include "system_state.h"

typedef enum
{
    BUTTON_START_PAUSE = 100,
    BUTTON_DIRECTION,
    BUTTON_SPEED

} ButtonType_t;

typedef struct
{
    uint8_t gpio;
    const char *name;
    ButtonType_t type;

} ButtonTaskParams_t;

void button_task(void *pvParameters);

#endif