#!/usr/bin/python3 -u

import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import logging

from superpower_i2c import SuperPower

# This short script logs the current temperature and battery voltage to MQTT in JSON-format.
# Change the following settings to your needs and add the following line to the
# crontab of the user pi (without the leading hash-sign):
# * * * * * /opt/superpower_daemon/superpower_daemon_mqtt_status.py

# Settings specific to MQTT
_topic = "topic"
_hostname = "localhost"
_port = 1883
_client_id = ""
_user = None
_password = None
#_additional_info = '"hostname" : "myhost"'
_additional_info = None

# Settings specific to SuperPower_Daemon
_time_const = 1.0   # used as a pause between i2c communications
# the number of retries when reading from or writing to the SuperPower_Daemon
_num_retries = 10
_i2c_address = 0x40  # the I2C address that is used for the SuperPower_Daemon

# Here begins the code


def get_uptime():
    with open('/proc/uptime', 'r') as f:
        uptime_seconds = float(f.readline().split()[0])
    return uptime_seconds


# set up logging
root_log = logging.getLogger()
root_log.setLevel("INFO")

# set up communication to the SuperPower_Daemon
bus = 1
SuperPower = SuperPower(bus, _i2c_address, _time_const, _num_retries)

# access data, an error is signalled by a return value of 0xFFFFFFFF/4294967295
temperature = str(SuperPower.get_temperature())
voltage = str(SuperPower.get_ups_bat_voltage())
uptime = str(get_uptime())

# build output
json_string = '{"temperature" : ' + temperature  \
              + ', "battery_voltage" : ' + voltage \
              + ', "uptime" : ' + uptime
if _additional_info == None:
    json_string = json_string + '}'
else:
    json_string = json_string + ', ' + _additional_info + '}'

# build auth dict
_auth = None
if _user != None:
    _auth = {'username': _user, 'password': _password}

# send data to MQTT
publish.single(_topic, payload=json_string, qos=0, retain=False, hostname=_hostname, port=_port,
               client_id=_client_id, keepalive=60, will=None, auth=_auth, tls=None, protocol=mqtt.MQTTv311, transport="tcp")
