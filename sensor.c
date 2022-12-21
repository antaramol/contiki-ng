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
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (5 * CLOCK_SECOND)   //Variable que envia mensajes cada segundo.

static struct simple_udp_connection udp_conn;
uint8_t Potencia;
uint8_t temp_ant=20;
uint8_t temp_min=10;
uint8_t temp_max=35;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
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
  Potencia=data[3];
  //printf("Potencia: %d\n",Potencia);
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
  uint8_t rec_pot;
  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      	rec_pot=Potencia-48;
      	
		printf("POTENCIA REC = %d\n",rec_pot);
		//Enfria si Potencia es igual a 1,2 o 3...
		if(rec_pot==1 || rec_pot==2 || rec_pot==3){
		
		  temp_ant=temp_ant-(4-rec_pot);
		
		} else if(rec_pot==4 || rec_pot==5 || rec_pot==6){ //Calienta si Potencia es igual a 4,5 o 6...
		
		  temp_ant=temp_ant+(rec_pot-3);
		
		}
		
		if(temp_ant<temp_min){
		temp_ant=temp_min;
		}
		else if(temp_ant>temp_max){
		temp_ant=temp_max;
		}
		
      int ent=temp_ant;
      //float frac= temp_ant-ent;
      //int dec=(frac*100);
      //printf("ent: %d, dec: %d\n",ent,dec);
      printf("\nEnviando temperatura actualizada al servidor = %d ºC\n",ent);
      sprintf(str, "%d", temp_ant);
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

