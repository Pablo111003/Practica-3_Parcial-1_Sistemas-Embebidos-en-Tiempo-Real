#include "buttons.h"
#include "app_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

typedef struct
{
    int stable_state;   /* último nivel confirmado (sin rebote) */
    int last_raw;       /* última lectura cruda del GPIO */
    int count;          /* contador de muestras consecutivas iguales */

} Debounce_t;

/* Número de muestras consecutivas iguales para confirmar un cambio de estado */
#define DEBOUNCE_COUNT 3

/* Devuelve true cuando se detecta un flanco de SOLTAR (rising edge),
   es decir, cuando el botón pasa de presionado (0) a libre (1).
   Con pull-up: no presionado = 1, presionado = 0. */
static bool debounce_update(uint8_t gpio, Debounce_t *db)
{
    int raw;
    bool pressed_event;

    raw = gpio_get_level((gpio_num_t)gpio);
    pressed_event = false;

    if (raw == db->last_raw)
    {
        /* La lectura coincide con la anterior: acumula confirmaciones */
        if (db->count < DEBOUNCE_COUNT)
        {
            db->count++;   /* corrección: era db->count-- en el original */
        }
    }
    else
    {
        /* Cambio detectado: reinicia el contador de confirmaciones */
        db->count = 0;
        db->last_raw = raw;
    }

    if (db->count >= DEBOUNCE_COUNT)
    {
        if (raw != db->stable_state)
        {
            db->stable_state = raw;

            /* Genera evento solo al SOLTAR el botón (nivel sube a 1).
               Esto evita disparos múltiples mientras se mantiene presionado. */
            if (db->stable_state == 1)
            {
                pressed_event = true;   /* corrección: era false en el original */
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

    /* Configura el GPIO como entrada con pull-up interno */
    gpio_reset_pin((gpio_num_t)cfg->gpio);
    gpio_set_direction((gpio_num_t)cfg->gpio, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)cfg->gpio, GPIO_PULLUP_ONLY);

    while (1)
    {
        if (debounce_update(cfg->gpio, &db))
        {
            /* El botón fue soltado después de un press válido.
               En lugar de actuar directamente, señalizamos al Task_Manager
               escribiendo el evento en la variable compartida. El manager
               leerá el evento en su próxima iteración y ejecutará la acción. */
            switch (cfg->type)
            {
                case BUTTON_START_PAUSE:
                    /* Solicita al manager que pause o reanude el conteo */
                    g_system.pending_event = MANAGER_EVENT_START_PAUSE;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_DIRECTION:
                    /* Solicita al manager que invierta la dirección */
                    g_system.pending_event = MANAGER_EVENT_DIRECTION;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_SPEED:
                    /* Solicita al manager que cambie la velocidad */
                    g_system.pending_event = MANAGER_EVENT_SPEED;
                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                default:
                    break;
            }
        }

        /* Muestrea el botón cada BUTTON_POLL_MS milisegundos */
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}
