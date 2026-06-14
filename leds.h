#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

typedef struct
{
    uint8_t gpio;
    uint8_t bit_position;
    const char *name;

} LedTaskParams_t;

void led_task(void *pvParameters);

#endif