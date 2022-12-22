#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "math.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "temperature-sensor.h"
#include "sys/etimer.h"
#include <stdio.h> /* For printf() */
#include "sys/log.h"
#include "dev/leds.h" /*Para controlar los leds*/
#include "dev/etc/rgb-led/rgb-led.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (5 * CLOCK_SECOND)   //Variable que envia mensajes cada segundo.

static struct simple_udp_connection udp_conn;
uint8_t potencia;
uint8_t temp_ant=20;
uint8_t temp_min=10;
uint8_t temp_max=35;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
PROCESS(parpadeo_process, "Hace parpadear el LED");
AUTOSTART_PROCESSES(&udp_client_process,&parpadeo_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  //printf("La potencia del nodo servidor es '%.*s'\n", datalen, (char *) data);
  
  //Consigo la potencia:
  potencia=data[0]-48;
  //printf("potencia: %d\n",potencia);
  //LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  //LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      	
	    printf("Potencia REC = %d\n",potencia);

        sprintf(str, "99");
        //printf("%s",str);
        simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);

        count++;
    } else {
      printf("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/



PROCESS_THREAD(parpadeo_process, ev, data)
{

  static struct etimer timer1;

  PROCESS_BEGIN();

  //PROCESS_WAIT_EVENT();

  etimer_set(&timer1, CLOCK_SECOND*3);
   

  while (1){
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));
    //printf("Potencia?\n");
    if (potencia != 0){
        if(1 <= potencia && potencia <= 3){
            printf("Ponemos el led a azul\n");
            rgb_led_set(RGB_LED_BLUE);
            switch(potencia){
                case 1:
                    etimer_set(&timer1, CLOCK_SECOND*0.25);
                    break;
                case 2:
                    etimer_set(&timer1, CLOCK_SECOND*0.5);
                    break;
                case 3:
                    etimer_set(&timer1, CLOCK_SECOND);
                    break;
            }
            	
		} else if(4 <= potencia && potencia <= 6){
            printf("Ponemos el red a rojo\n");
            rgb_led_set(RGB_LED_RED);
            switch (potencia) {
                case 4:
                    etimer_set(&timer1, CLOCK_SECOND);
                    break;
                case 5:
                    etimer_set(&timer1, CLOCK_SECOND*0.5);
                    break;
                case 6:
                    etimer_set(&timer1, CLOCK_SECOND*0.25);
                    break;
            }
            
		}
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));
        etimer_reset(&timer1);
    }else{
        printf("Led apagado\n");
        etimer_set(&timer1, CLOCK_SECOND);
        
    }
    rgb_led_off();
  }


  PROCESS_END();
}

/**********************************************************/

