/**
 * =============================================================================
 * CONCLUSION DEL EQUIPO
 * Integrantes: Pablo Mansilla Hernández
 *
 * Al realizar este codigo, pude comprender como es que, utilizando FreeRTOS, se puede dividir un sistema 
 * en tareas independientes que corren de forma concurrente, donde cada una
 * de las tareas, tiene una asignación diferente. Cuando alguno de los botones es pulsado, se notifica 
 * al manager mediante un evento en la variable global. A pesar de que los tres botones utilizan la 
 * misma función "button_task", el struct hace que se diferencien, recibiendo uno diferente en 
 * "pvParameters". De este modo, una sola función puede realizar las tres funciones sin la necesidad 
 * de duplicar codigo.
 * 
 * 
 * ¿Qué diferencia existe entre BLOCKED y SUSPENDED?
 * R= Mientras que BLOCKED es temporal y automatico, SUSPENDED es manual y permanente, con BLOCKED la 
 * tarea sola decide cuándo salir de ese estado, ya sea porque venció un tiempo o porque llegó un dato 
 * que esperaba. Con SUSPENDED en cambio, la tarea no puede salir sola, alguien externo tiene que 
 * llamar "vTaskResume()" sobre ella.
 * 
 * 
 * =============================================================================
 */



#include "system_state.h"
#include "app_task.h"

void app_main(void)
{
    // Inicializa la estructura global con los valores por defecto:
    system_state_init();

    // Crea todas las tareas FreeRTOS
    app_tasks_create();
}
