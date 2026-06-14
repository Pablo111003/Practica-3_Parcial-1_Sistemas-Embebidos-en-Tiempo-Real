#include "app_task.h"

#include "app_config.h"
#include "system_state.h"
#include "leds.h"
#include "buttons.h"
#include "counter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "MANAGER";

/* Handles de las tareas; el manager los usa para suspender/reanudar */
static TaskHandle_t h_leds[4];
static TaskHandle_t h_btn_start;
static TaskHandle_t h_btn_dir;
static TaskHandle_t h_btn_speed;
static TaskHandle_t h_counter;
static TaskHandle_t h_manager;

/* Parámetros de cada tarea LED.
   Cada LED representa un bit del valor BCD:
     LED_B0 → bit 0 (valor 1)
     LED_B1 → bit 1 (valor 2)
     LED_B2 → bit 2 (valor 4)
     LED_B3 → bit 3 (valor 8)  */
static LedTaskParams_t led_params[4] =
{
    { .gpio = LED_B0, .bit_position = 0, .name = "LED_B0" },
    { .gpio = LED_B1, .bit_position = 1, .name = "LED_B1" },
    { .gpio = LED_B2, .bit_position = 2, .name = "LED_B2" },
    { .gpio = LED_B3, .bit_position = 3, .name = "LED_B3" },
};

/* Parámetros de los tres botones */
static ButtonTaskParams_t btn_start =
{
    .gpio = BTN_START,
    .name = "BTN_START",
    .type = BUTTON_START_PAUSE
};

static ButtonTaskParams_t btn_dir =
{
    .gpio = BTN_DIR,
    .name = "BTN_DIR",
    .type = BUTTON_DIRECTION
};

static ButtonTaskParams_t btn_speed =
{
    .gpio = BTN_SPEED,
    .name = "BTN_SPEED",
    .type = BUTTON_SPEED
};

/* Convierte el estado de una tarea FreeRTOS a texto legible.
   Corrección del original: los strings "SUSPENDED" y "RUNNING" estaban
   intercambiados. */
static const char *state_to_string(eTaskState state)
{
    switch (state)   /* corrección: el parámetro es 'state', no 'eTaskState' */
    {
        case eRunning:
            return "RUNNING";      /* corrección: estaba "SUSPENDED" */

        case eReady:
            return "READY";

        case eBlocked:
            return "BLOCKED";

        case eSuspended:
            return "SUSPENDED";    /* corrección: estaba "RUNNING" */

        case eDeleted:
            return "DELETED";

        default:
            return "UNKNOWN";
    }
}

/* Pausa el sistema: suspende el contador y los botones de dirección/velocidad.
   El botón START/PAUSE sigue activo para poder reanudar. */
static void manager_pause_system(void)
{
    g_system.mode = SYSTEM_PAUSED;

    /* Suspende el contador; conserva su valor actual y no da más pasos.
       Solo puede reanudarse con vTaskResume(). */
    vTaskSuspend(h_counter);

    /* Suspende los botones de dirección y velocidad.
       En pausa no tiene sentido cambiarlos. */
    vTaskSuspend(h_btn_dir);
    vTaskSuspend(h_btn_speed);

    ESP_LOGW(TAG, "Sistema PAUSADO");
}

static void manager_run_system(void)
{
    g_system.mode = SYSTEM_RUNNING;

    /* Primero reanuda los botones de control */
    vTaskResume(h_btn_dir);
    vTaskResume(h_btn_speed);

    /* Luego reanuda el contador; continúa desde donde se quedó */
    vTaskResume(h_counter);

    ESP_LOGW(TAG, "Sistema RUNNING");
}

/* Invierte la dirección de conteo.
   Corrección del original: el else también asignaba COUNT_DOWN en lugar de COUNT_UP. */
static void manager_toggle_direction(void)
{
    if (g_system.direction == COUNT_UP)
    {
        g_system.direction = COUNT_DOWN;
    }
    else
    {
        g_system.direction = COUNT_UP;   /* corrección: era COUNT_DOWN */
    }

    ESP_LOGI(TAG, "Nueva direccion: %s",
             g_system.direction == COUNT_UP ? "UP" : "DOWN");
}

/* Alterna la velocidad entre lenta (500 ms) y rápida (250 ms).
   Corrección del original: el else también asignaba SPEED_FAST_MS. */
static void manager_toggle_speed(void)
{
    if (g_system.period_ms == SPEED_SLOW_MS)
    {
        g_system.period_ms = SPEED_FAST_MS;
    }
    else
    {
        g_system.period_ms = SPEED_SLOW_MS;   /* corrección: era SPEED_FAST_MS */
    }

    ESP_LOGI(TAG, "Nueva velocidad: %lu ms", (unsigned long)g_system.period_ms);
}

/* Imprime el estado de cada tarea y el estado global del sistema */
static void manager_print_states(void)
{
    ESP_LOGI(TAG, "------ ESTADOS ------");

    ESP_LOGI(TAG, "COUNTER: %s",   state_to_string(eTaskGetState(h_counter)));
    ESP_LOGI(TAG, "BTN_START: %s", state_to_string(eTaskGetState(h_btn_start)));
    ESP_LOGI(TAG, "BTN_DIR: %s",   state_to_string(eTaskGetState(h_btn_dir)));
    ESP_LOGI(TAG, "BTN_SPEED: %s", state_to_string(eTaskGetState(h_btn_speed)));

    for (int i = 0; i < 4; i++)
    {
        ESP_LOGI(TAG, "%s: %s",
                 led_params[i].name,
                 state_to_string(eTaskGetState(h_leds[i])));
    }

    ESP_LOGI(TAG,
             "Valor=%u | Modo=%s | Direccion=%s | Periodo=%lu ms",
             g_system.value,
             g_system.mode == SYSTEM_RUNNING ? "RUNNING" : "PAUSED",
             g_system.direction == COUNT_UP ? "UP" : "DOWN",
             (unsigned long)g_system.period_ms);
}

static void task_manager(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_print;
    last_print = xTaskGetTickCount();

    while (1)
    {
        /* Lee el evento pendiente que los botones pudieron haber escrito */
        ManagerEvent_t events = g_system.pending_event;   /* corrección: 'events' en lugar de 'event' */

        if (events != MANAGER_EVENT_NONE)
        {
            /* Consume el evento: lo limpia para que no se procese dos veces */
            g_system.pending_event = MANAGER_EVENT_NONE;

            switch (events)   /* corrección: usar 'events' consistentemente */
            {
                case MANAGER_EVENT_SPEED:
                    /* Cambio de velocidad solo si el sistema está corriendo.
                       Corrección: ambas ramas llamaban a manager_run_system(). */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_speed();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Velocidad ignorada: sistema pausado");
                    }
                    break;

                case MANAGER_EVENT_DIRECTION:
                    /* Cambio de dirección solo si el sistema está corriendo */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_direction();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Direccion ignorada: sistema pausado");
                    }
                    break;   /* corrección: faltaba el break en el original */

                case MANAGER_EVENT_START_PAUSE:
                    /* Alterna entre RUNNING y PAUSED.
                       Corrección: ambas ramas llamaban a manager_toggle_speed(). */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_pause_system();
                    }
                    else
                    {
                        manager_run_system();
                    }
                    break;

                default:
                    break;
            }
        }

        /* Log periódico de estados cada 2 segundos */
        if ((xTaskGetTickCount() - last_print) >= pdMS_TO_TICKS(2000))
        {
            last_print = xTaskGetTickCount();
            manager_print_states();
        }

        /* El manager revisa eventos cada 20 ms (no cada 20000 ms como en el original) */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_tasks_create(void)
{
    /* --- Crea las 4 tareas de LEDs ---
       Cada una maneja un solo pin GPIO y refleja un bit del valor BCD.
       Prioridad 1 (baja): solo actualizan pines, no son críticas de tiempo. */
    for (int i = 0; i < 4; i++)
    {
        xTaskCreate(led_task,
                    led_params[i].name,
                    2048,
                    (void *)&led_params[i],
                    1,
                    &h_leds[i]);
    }

    /* --- Crea las 3 tareas de botones ---
       Prioridad 2: deben responder al usuario antes de que el manager actúe. */
    xTaskCreate(button_task, "BTN_START", 2048, (void *)&btn_start, 2, &h_btn_start);
    xTaskCreate(button_task, "BTN_DIR",   2048, (void *)&btn_dir,   2, &h_btn_dir);
    xTaskCreate(button_task, "BTN_SPEED", 2048, (void *)&btn_speed, 2, &h_btn_speed);

    /* --- Crea la tarea del contador ---
       Prioridad 2: misma que los botones; el manager la suspende/reanuda. */
    xTaskCreate(counter_task, "COUNTER", 2048, NULL, 2, &h_counter);

    /* --- Crea la tarea del manager ---
       Prioridad 3 (la más alta): debe procesar eventos antes que las demás. */
    xTaskCreate(task_manager, "MANAGER", 2048, NULL, 3, &h_manager);

    /* Estado inicial: sistema pausado.
       Suspende el contador y los botones de dirección/velocidad.
       El botón START/PAUSE queda activo para iniciar el conteo. */
    manager_pause_system();
}
