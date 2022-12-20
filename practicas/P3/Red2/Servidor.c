#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (5 * CLOCK_SECOND)   //Variable que envia mensajes cada segundo.

static struct simple_udp_connection udp_conn;

static uint8_t temp_actual=25; //Definicion de la temperatura inicial.
static uint8_t potencia;
static uint8_t* p_potencia = &potencia;
const char *gonzalo = "fd00::f6ce:36a7:3096:489";
const char *juanma = "fd00::f6ce:3694:f16:1753";



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
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  /**if (sender_addr== IPsenor){
    temp_actual = data;
  }else if(sender_addr==IPactuador){
    data = potencia;
  }

  gonzalo = fd00::f6ce:36a7:3096:489
  juanma = fd00::f6ce:3694:f16:1753
  **/
  //static uint8_t temperatura = 0;

  if (sender_addr==(uip_ipaddr_t*)juanma){
    printf("Temperatura: %s\n",(char*)data);  
  }else{
    printf("No es juanma\n");
  }
  

#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  //snprintf((char *)data,"%d.%d")
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, p_potencia, datalen, sender_addr);
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

/*--------------------------------------------------------*/
PROCESS_THREAD(calculo_potencia, ev, data)
{
  static struct etimer timer1;
  static uint8_t temp_deseada=0;
  static uint8_t diff = 0;

  PROCESS_BEGIN();
  
  while(1){
    
    etimer_set(&timer1, CLOCK_SECOND*10); //Ajustar esto
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer1));

    temp_deseada=temp_actual + rand()%18 - 9;
    printf("Temperatura deseada: %d\n",temp_deseada);
    diff = temp_deseada-temp_actual;
    
    if(diff >0){
      if(diff <2){
        potencia = 3;
      }else if(diff<7){
        potencia = 2;
      }else{
        potencia = 1;
      }
    }else if(diff>-2){
      potencia = 4;
    }else if(diff>-7){
      potencia = 5;
    }else{
      potencia = 6;
    }
    
    printf("Potencia: %d\n",potencia);


  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

