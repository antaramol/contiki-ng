#include "contiki.h"
#include "dev/leds.h" /*Para controlar los leds*/
#include "dev/etc/rgb-led/rgb-led.h"
#include <stdio.h> 
/*---------------------------------------------------------------------------*/
PROCESS(timer_process, "Proceso inicial que lanza el resto");
PROCESS(parpadeo_1_process, "Conmuta el LED1");
PROCESS(parpadeo_2_process, "Conmuta el LED2");
AUTOSTART_PROCESSES(&timer_process, &parpadeo_1_process, &parpadeo_2_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(timer_process, ev, data)
{
  static struct etimer timer1;

  PROCESS_BEGIN();
  
  etimer_set(&timer1, CLOCK_SECOND*3);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));

  process_poll(&parpadeo_1_process);
  process_poll(&parpadeo_2_process);
  
  
  PROCESS_END();
}

PROCESS_THREAD(parpadeo_1_process, ev, data)
{

  static struct etimer timer2;

  PROCESS_BEGIN();
  
  PROCESS_WAIT_EVENT();

  etimer_set(&timer2, CLOCK_SECOND*2);
  
  
  while(1) {
    leds_toggle(LEDS_GREEN);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer2));
    etimer_reset(&timer2);
  }

  PROCESS_END();
}


PROCESS_THREAD(parpadeo_2_process, ev, data)
{

  static struct etimer timer3;

  PROCESS_BEGIN();

  PROCESS_WAIT_EVENT();

  etimer_set(&timer3, CLOCK_SECOND*4);
   

  while(1) {
    leds_toggle(LEDS_YELLOW);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer3));
    etimer_reset(&timer3);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
