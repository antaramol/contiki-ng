#include "contiki.h"
#include "temperature-sensor.h"
#include <stdio.h> 
/*---------------------------------------------------------------------------*/
PROCESS(timer_process, "Cuenta 3 segundos");
PROCESS(leer_temperatura, "Lee el valor del sensor");
AUTOSTART_PROCESSES(&timer_process, &leer_temperatura);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(timer_process, ev, data)
{
  static struct etimer timer1;

  PROCESS_BEGIN();
  
  etimer_set(&timer1, CLOCK_SECOND*3);


  while(1){
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));
    process_poll(&leer_temperatura);
    etimer_reset(&timer1);

  }
  
  PROCESS_END();
}

PROCESS_THREAD(leer_temperatura, ev, data)
{

  PROCESS_BEGIN();
  static int valor = 0;
  PROCESS_WAIT_EVENT();

  
  while(1) {
    SENSORS_ACTIVATE(temperature_sensor);
    valor = temperature_sensor.value(0);
    printf("temperatura = %d",valor);
    SENSORS_DEACTIVATE(temperature_sensor);
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
