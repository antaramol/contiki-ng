#!/usr/bin/env python3

""" MQTT exporter """

import json
#from msilib.schema import ServiceInstall
import re
import signal
import logging
import os
import sys
import serial

import paho.mqtt.client as mqtt
from prometheus_client import Counter, Gauge, start_http_server

logging.basicConfig(filename='register.log', level=logging.DEBUG)
LOG = logging.getLogger("[mqtt-exporter]")

prom_msg_counter = None
prom_pot_gauge = None
prom_tempAct_gauge = None
prom_tempDeseada_gauge = None

def create_msg_counter_metrics():
    global prom_msg_counter

    prom_msg_counter = Counter( 'number_msgs',
        'Number of received messages'
    )

def create_pot_gauge_metrics():
    global prom_pot_gauge

    prom_pot_gauge = Gauge( 'potencia',
        'Potencia[Niveles acordados]'
    )

def create_tempAct_gauge_metrics():
    global prom_tempAct_gauge

    prom_tempAct_gauge = Gauge( 'tempAct',
        'Temperatura [Nivel actual]'
    )

def create_tempDeseada_gauge_metrics():
    global prom_tempDeseada_gauge

    prom_tempDeseada_gauge = Gauge( 'tempDeseada',
        'Temperatura [Nivel deseado]'
    )

def parse_message(raw_topic, raw_payload):
    try:
        payload = json.loads(raw_payload)
    except json.JSONDecodeError:
        LOG.error(" Failed to parse payload as JSON: %s", str(payload, 'ascii'))
        return None, None
    except UnicodeDecodeError:
        LOG.error(" Encountered undecodable payload: %s", raw_payload)
        return None, None
    
    topic = raw_topic

    return topic, payload

def parse_metric(data):
    if isinstance(data, (int,float)):
        return data
    
    if isinstance(data, bytes):
        data = data.decode()

    if isinstance(data, str):
        data = data.upper()

    return float(data)

def parse_metrics(data, topic, client_id):
    for metric, value in data.items():
        if isinstance(value, dict):
            LOG.debug(" Parsing dict %s, %s", metric, value)
            parse_metrics(value, topic, client_id)
            continue

        try:
            metric_value = parse_metric(value)
        except ValueError as err:
            LOG.error(" Failed to convert %s, Error: %s", metric, err)



def on_connect(client, _, __, rc):
    LOG.info(" Connected with result code: %s", rc)
    client.subscribe("Potencia")
    client.subscribe("tempAct")
    client.subscribe("tempDeseada")
    if rc != mqtt.CONNACK_ACCEPTED:
        LOG.error("[ERROR]: MQTT %s", rc)

def on_message(_, userdata, msg):
    LOG.info(" [Topic: %s] %s", msg.topic, msg.payload)

    topic, payload = parse_message(msg.topic, msg.payload)
    LOG.debug(" \t Topic: %s", topic)
    LOG.debug(" \t Payload: %s", payload)

    if not topic or not payload:
        LOG.error(" [ERROR]: Topic or Payload not found")
        return

    prom_msg_counter.inc()
    if (topic == "Potencia"):
        prom_pot_gauge.set(payload)
    elif (topic == "tempAct"):
        prom_tempAct_gauge.set(payload)
    elif (topic == "tempDeseada"):
        prom_tempDeseada_gauge.set(payload)

def main():
    # LOG.debug("Hello world")
    # Create MQTT client
    client = mqtt.Client()

    def stop_reques(signum, frame):
        LOG.debug(" Stopping MQTT exporter")
        client.disconnect()
        ser.close()
        sys.exit(0)

    #Serial port
    try:
        ser = serial.Serial(port="/dev/ttyACM0",
                            baudrate=115200,
                            parity=serial.PARITY_NONE,
                            stopbits=serial.STOPBITS_ONE,
                            bytesize=serial.EIGHTBITS,
                            timeout=2000)
        ser.flushInput()
        ser.flush()
        ser.isOpen()
        LOG.info("Serial Port /dev/ACM0 is opened")
    except IOError:
        LOG.error("serial port is already opened or does not exist")
        sys.exit(0)

    # Create Prometheus metrics
    create_msg_counter_metrics()
    create_pot_gauge_metrics()
    create_tempAct_gauge_metrics()
    create_tempDeseada_gauge_metrics()
    # Start prometheus server
    start_http_server(9000)

    # Configure MQTT topic
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Suscribe MQTT topics
    LOG.debug(" Connecting to localhost")
    #client.connect(os.getenv("MQTT_ADDRESS", "localhost"), 1883, 60)
    #client.connect("mosquitto", 1883, 60)
    client.connect("localhost", 1883, 60)
    # Waiting for messages
    #client.loop_forever()
    client.loop_start()

    while True:
        line = ser.readline()
        LOG.debug("Serial Data: %s", str(line, 'ascii').rstrip())

        ser_pot=int(int(line.rstrip()[0]) - 48)
        temp_act=10*(int(line.rstrip()[2]) - 48)+ int(line.rstrip()[3]) - 48
        temp_des=10*(int(line.rstrip()[5]) - 48)+ int(line.rstrip()[6]) - 48
        print(ser_pot)
        client.publish(topic="Potencia", payload=ser_pot, qos=0, retain=False)
        client.publish(topic="tempAct", payload=temp_act, qos=0, retain=False)
        client.publish(topic="tempDeseada", payload=temp_des, qos=0, retain=False)

if __name__ == "__main__":
    main()