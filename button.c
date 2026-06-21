#include "buttons.h"
#include "app_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

typedef struct
{
    int stable_state;   // último nivel confirmado
    int last_raw;       // última lectura cruda del GPIO
    int count;          // contador de muestras consecutivas iguales

} Debounce_t;

// Número de muestras consecutivas iguales para confirmar un cambio de estado
#define DEBOUNCE_COUNT 3

static bool debounce_update(uint8_t gpio, Debounce_t *db)
{
    int raw;
    bool pressed_event;

    raw = gpio_get_level((gpio_num_t)gpio);
    pressed_event = false;

    if (raw == db->last_raw)
    {
        // Si la lectura coincide con la anterior, se acumulan confirmaciones
        if (db->count < DEBOUNCE_COUNT)
        {
            db->count++; 
        }
    }
    else
    {
        // Si se detecta un cambio, reinicia el contador de confirmaciones
        db->count = 0;
        db->last_raw = raw;
    }

    if (db->count >= DEBOUNCE_COUNT)
    {
        if (raw != db->stable_state)
        {
            db->stable_state = raw;

            if (db->stable_state == 1)
            {
                pressed_event = true;
            }
        }
    }

    return pressed_event;
}

void button_task(void *pvParameters)
{
    ButtonTaskParams_t *cfg = (ButtonTaskParams_t *)pvParameters;

    Debounce_t db =
    {
        .stable_state = 1,
        .last_raw     = 1,
        .count        = 0
    };

    // Configura el GPIO como entrada con pull-up interno
    gpio_reset_pin((gpio_num_t)cfg->gpio);
    gpio_set_direction((gpio_num_t)cfg->gpio, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)cfg->gpio, GPIO_PULLUP_ONLY);

    while (1)
    {
        if (debounce_update(cfg->gpio, &db))
        {
            switch (cfg->type)
            {
                case BUTTON_START_PAUSE:
                    // Solicita al manager que pause o reanude el conteo
                    g_system.pending_event = MANAGER_EVENT_START_PAUSE;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_DIRECTION:
                    // Solicita al manager que invierta la dirección
                    g_system.pending_event = MANAGER_EVENT_DIRECTION;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_SPEED:
                    // Solicita al manager que cambie la velocidad
                    g_system.pending_event = MANAGER_EVENT_SPEED;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                default:
                    break;
            }
        }

        // Muestrea el botón cada ciertos milisegundos
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}
