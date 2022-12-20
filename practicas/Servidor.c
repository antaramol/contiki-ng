#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "temperature-sensor.h"
#include "sys/etimer.h"
#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)   //Variable que envia mensajes cada segundo.

static struct simple_udp_connection udp_conn;

uint8_t temp_actual=25; //Definicion de la temperatura inicial.
static uint8_t potencia;


PROCESS(udp_server_process, "UDP server");
PROCESS(calculo_potencia, "Power");
AUTOSTART_PROCESSES(&udp_server_process,&calculo_potencia);
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
  printf("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  printf("\n");
  if (sender_addr== /*IPsenor*/){
    temp_actual = data;
  }else if(sender_addr==/*IPactuador*/){
    data = potencia;
  }

#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  printf("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}

PROCESS_THREAD(calculo_potencia, ev, data)
{
  static struct etimer timer1;
  static uint8_t temp_deseada=temp_actual + rand()%18 - 9; //Definicion de la temperatura deseada.
  static diff = 0;

  PROCESS_BEGIN();
  
  while(1){
    
    etimer_set(&timer1, CLOCK_SECOND*3); //Ajustar esto
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));

    temp_deseada=temp_actual + rand()%18 - 9;
    
    if(diff >0){
      if(diff <2){
        potencia = 2;
      }else if(diff<7){
        potencia = 1;
      }else{
        potencia = 0;
      }
    }else if(diff>-2){
      potencia = 3;
    }else if(diff>-7){
      potencia = 4;
    }else{
      potencia = 5;
    }
    

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

