#include "contiki.h"
#include "temperature-sensor.h"
#include <stdio.h> 
#include "dev/gpio-hal.h"
#include "sys/etimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"

#include <inttypes.h>

/*****************************************************************************/
extern gpio_hal_pin_t btn_pin;
static bool boton = 0;

#define btn_port    GPIO_HAL_NULL_PORT

/*---------------------------------------------------------------------------*/
PROCESS(timer_process, "Cuenta 3 segundos");
PROCESS(leer_temperatura, "Lee el valor del sensor");
PROCESS(boton_process, "Boton");
AUTOSTART_PROCESSES(&timer_process, &leer_temperatura, &boton_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(timer_process, ev, data)
{
  static struct etimer timer1;

  PROCESS_BEGIN();
  
  etimer_set(&timer1, CLOCK_SECOND*2);


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
    if(ev == PROCESS_EVENT_POLL){
      SENSORS_ACTIVATE(temperature_sensor);
      valor = temperature_sensor.value(0)/4.0;
      valor_entero = (int)valor;
      valor_dec = (int)((valor - valor_entero)*100.0);
      printf("%02d.%02d,%d\n",valor_entero,valor_dec,boton);
      SENSORS_DEACTIVATE(temperature_sensor);  }
    }
    

  PROCESS_END();
}

PROCESS_THREAD(boton_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == button_hal_press_event) {
      //printf("Boton: %d\n",i);
      boton = !boton;
    }
  }
  PROCESS_END();
}