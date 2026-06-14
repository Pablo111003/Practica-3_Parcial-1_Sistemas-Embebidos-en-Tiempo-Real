# Nombres: Pablo Mansilla Hernández (9129)

# Descripción del Codigo:

El siguiente código consiste en un sistema multitask, utilizando FreeRTOS en una tarjeta ESP32, donde se implementa un contador BCD el cual puede ascender y descender (0-9), 
el contador es controlado a traves de tres botones, el primer boton inicia o pausa el conteo, el segundo boton cambia la dirección del conteo, y el tercer boton cambia la velocidad
entre 500 ms, y 250 ms. El valor del contador es reflejado a traves de 4 leds que representan los numeros en binario desde el 0 hasta el 9.

# Tabla de pines:

GPIO 02 -> LED B0 (bit 0) -> Resistencia 220 Ohms -> GND

GPIO 04 -> LED B1 (bit 1) -> Resistencia 220 Ohms -> GND

GPIO 05 -> LED B2 (bit 2) -> Resistencia 220 Ohms -> GND

GPIO 18 -> LED B3 (bit 3) -> Resistencia 220 Ohms -> GND

GPIO 13 -> Boton Dirección -> GND

GPIO 14 -> Boton Velocidad -> GND

GPIO 15 -> Boton Inicio -> GND

# Conclusiones:

Al realizar este codigo, pude comprender como es que, utilizando FreeRTOS, se puede dividir un sistema en tareas independientes que corren de forma concurrente, donde cada una
de las tareas, tiene una asignación diferente. Cuando alguno de los botones es pulsado, se notifica al manager mediante un evento en la variable global. A pesar de que los tres
botones utilizan la misma función "button_task", el struct hace que se diferencien, recibiendo uno diferente en "pvParameters". De este modo, una sola función puede realizar las
tres funciones sin la necesidad de duplicar codigo.


# Preguntas:

1. ¿Qué diferencia existe entre BLOCKED y SUSPENDED?
   R= Mientras que BLOCKED es temporal y automatico, SUSPENDED es manual y permanente, con BLOCKED la tarea sola decide cuándo salir de ese estado, ya sea porque venció un tiempo
   o porque llegó un dato que esperaba. Con SUSPENDED en cambio, la tarea no puede salir sola, alguien externo tiene que llamar "vTaskResume()" sobre ella.

2. ¿Qué función cumple el Idle Task?
   R= Idle Task es una tarea especial que FreeRTOS crea automáticamente con la prioridad más baja posible. Solamente se ejecuta cuando todas las tareas están BLOCKED o SUSPENDED.
   Su función principal es liberar la memoria de tareas que fueron eliminadas con "vTaskDelete()"

3. ¿Cómo decide FreeRTOS cuál tarea ejecutar cuando varias tienen la misma prioridad?
   R= Si hay varias tareas con la misma prioridad, FreeRTOS utiliza Round Robin, dandole a todas un tick (1 ms) para usar el CPU por turno
   
