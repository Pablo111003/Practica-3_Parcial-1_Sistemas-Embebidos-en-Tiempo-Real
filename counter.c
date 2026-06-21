#include "counter.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "COUNTER";

// Avanza o retrocede el contador BCD (0–9) según la dirección actual.
static void counter_step(void)
{
    // Si la dirección es ascendente, incrementa
    if (g_system.direction == COUNT_UP)
    {
        g_system.value = (g_system.value >= 9) ? 0 : g_system.value + 1;
    }
    else
    // Si la dirección es descendente, decrementa
    {
        g_system.value = (g_system.value == 0) ? 9 : g_system.value - 1;
    }
}

void counter_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        // Log del estado actual antes de avanzar. Si la tarea está suspendida por el manager, no llega a esta parte
        ESP_LOGI(TAG, "Valor=%u | Direccion=%s | Periodo=%lu ms",
                 g_system.value,
                 g_system.direction == COUNT_UP ? "UP" : "DOWN",
                 (unsigned long)g_system.period_ms);

        // Espera el tiempo del periodo configurado (500 ms lento / 250 ms rápido) antes de dar el siguiente paso
        vTaskDelay(pdMS_TO_TICKS(g_system.period_ms));

        // Incrementa o decrementa el valor BCD
        counter_step();
    }
}
