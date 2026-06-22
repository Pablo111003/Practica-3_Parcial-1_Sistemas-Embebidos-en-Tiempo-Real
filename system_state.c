#include "system_state.h"
#include "app_config.h"

// Variable global que contiene el estado completo del sistema. Todas las tareas la leen/escriben para coordinar el comportamiento.
SystemState_t g_system;

void system_state_init(void)
{
    // El contador arranca en cero
    g_system.value = 0;

    // Velocidad inicial: lento (500 ms)
    g_system.period_ms = SPEED_SLOW_MS;

    // Dirección inicial: ascendente (0 → 9)
    g_system.direction = COUNT_UP;

    // Modo inicial: pausado. El sistema NO cuenta hasta que se presione START por primera vez
    g_system.mode = SYSTEM_PAUSED;

    // Sin eventos pendientes al inicio
    g_system.pending_event = MANAGER_EVENT_NONE;
}
