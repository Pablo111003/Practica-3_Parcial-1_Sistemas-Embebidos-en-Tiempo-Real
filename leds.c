#include "leds.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

void led_task(void *pvParameters)
{
    LedTaskParams_t *cfg = (LedTaskParams_t *)pvParameters;

    // Configura el pin del LED como salida y lo apaga al inicio
    gpio_reset_pin((gpio_num_t)cfg->gpio);
    gpio_set_direction((gpio_num_t)cfg->gpio, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)cfg->gpio, 0);

    while (1)
    {
        uint8_t bit_value;

        // Extrae el bit correspondiente a este LED del valor BCD actual.
        bit_value = (g_system.value >> cfg->bit_position) & 0x01;

        // Envía el bit extraído al GPIO: 1 = LED encendido, 0 = LED apagado
        gpio_set_level((gpio_num_t)cfg->gpio, bit_value);

        // Actualiza el LED cada 20 ms; la tarea queda BLOCKED entre ciclos
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
