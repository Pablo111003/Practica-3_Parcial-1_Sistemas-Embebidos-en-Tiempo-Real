#include "system_state.h"
#include "app_config.h"

/* Variable global que contiene el estado completo del sistema.
   Todas las tareas la leen/escriben para coordinar el comportamiento. */
SystemState_t g_system;

void system_state_init(void)
{
    /* 1. El contador arranca en cero (0000 en binario = ningún LED encendido) */
    g_system.value = 0;

    /* 2. Velocidad inicial: lento (500 ms por paso) */
    g_system.period_ms = SPEED_SLOW_MS;

    /* 3. Dirección inicial: ascendente (0 → 9) */
    g_system.direction = COUNT_UP;

    /* 4. Modo inicial: pausado.
       El sistema NO cuenta hasta que se presione START por primera vez. */
    g_system.mode = SYSTEM_PAUSED;

    /* 5. Sin eventos pendientes al inicio */
    g_system.pending_event = MANAGER_EVENT_NONE;
}
