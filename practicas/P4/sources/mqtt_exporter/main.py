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
prom_temp_F_gauge = None
prom_temp_C_gauge = None
prom_switch_gauge = None

def create_msg_counter_metrics():
    global prom_msg_counter

    prom_msg_counter = Counter( 'number_msgs',
        'Number of received messages'
    )

def create_temp_F_gauge_metrics():
    global prom_temp_F_gauge

    prom_temp_F_gauge = Gauge( 'temp_F',
        'Temperature [Fahrenheit]'
    )

def create_temp_C_gauge_metrics():
    global prom_temp_C_gauge

    prom_temp_C_gauge = Gauge( 'temp_C',
        'Temperature [Celsius]'
    )

def create_switch_gauge_metrics():
    global prom_switch_gauge

    prom_switch_gauge = Gauge( 'switch',
        'Switch [On/Off]'
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
    client.subscribe("temp_F")
    client.subscribe("temp_C")
    client.subscribe("switch")
    if rc != mqtt.CONNACK_ACCEPTED:
        LOG.error("[ERROR]: MQTT %s", rc)

def on_message(_, userdata, msg):
    LOG.info(" [Topic: %s] %s", msg.topic, msg.payload)

    topic, payload = parse_message(msg.topic, msg.payload)
    LOG.debug(" \t Topic: %s", topic)
    LOG.debug(" \t Payload: %s", payload)

    if not topic or not str(payload):
        LOG.error(" [ERROR]: Topic or Payload not found")
        return

    prom_msg_counter.inc()
    if(topic == "temp_F"):
        prom_temp_F_gauge.set(payload)
    elif(topic == "temp_C"):
        prom_temp_C_gauge.set(payload)
    elif(topic == "switch"):
        prom_switch_gauge.set(payload)


def main():
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
    create_temp_F_gauge_metrics()
    create_temp_C_gauge_metrics()
    create_switch_gauge_metrics()
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

        
        temp_C = float(line.rstrip()[0:5])
        temp_F = temp_C*9/5+32
        boton = int(line.rstrip()[6]) - 48
        LOG.info(temp_C)
        client.publish(topic="temp_F", payload=temp_F, qos=0, retain=False)
        client.publish(topic="temp_C", payload=temp_C, qos=0, retain=False)
        client.publish(topic="switch", payload=boton, qos=0, retain=False)

if __name__ == "__main__":
    main()