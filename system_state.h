#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SYSTEM_PAUSED = 50,
    SYSTEM_RUNNING

} SystemMode_t;

typedef enum
{
    COUNT_UP = 0,
    COUNT_DOWN

} CountDirection_t;

typedef enum
{
    MANAGER_EVENT_NONE = 0,
    MANAGER_EVENT_START_PAUSE,
    MANAGER_EVENT_DIRECTION,
    MANAGER_EVENT_SPEED

} ManagerEvent_t;

typedef struct
{
    volatile uint8_t value;
    volatile uint32_t period_ms;
    volatile CountDirection_t direction;
    volatile SystemMode_t mode;

    volatile ManagerEvent_t pending_event;

} SystemState_t;

extern SystemState_t g_system;

void system_state_init(void);

#endif