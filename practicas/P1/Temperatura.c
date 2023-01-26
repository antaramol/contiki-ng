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
  static float valor = 0;
  static int valor_entero, valor_dec = 0;


  
  while(1) {

    PROCESS_WAIT_EVENT();

    SENSORS_ACTIVATE(temperature_sensor);
    valor = temperature_sensor.value(0)/4.0;
    valor_entero = (int)valor;
    valor_dec = (int)((valor - valor_entero)*100.0);
    printf("%d.%d\n",valor_entero,valor_dec);
    SENSORS_DEACTIVATE(temperature_sensor);  }

  PROCESS_END();
}
