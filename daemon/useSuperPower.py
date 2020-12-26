#!/usr/bin/env python3 

import sys

#sys.path.append('/opt/superpowerdaemon/')  # add the path to our SuperPower module

from ctypes import *
import time
import logging
from superpower_i2c import SuperPower

_time_const  = 0.1  # used as a pause between i2c communications
_num_retries = 1   # the number of retries when reading from or writing to the SuperPower UPS
_i2c_address = 0x40 # the I2C address that is used for the SuperPower UPS

# Data structures for the charger status
class ChargerStatusVal(Structure):
    _fields_ = [("VSYS_STAT", c_uint8, 1),
                ("SDP_STAT", c_uint8, 1),
                ("PG_STAT", c_uint8, 1),
                ("CHRG_STAT", c_uint8, 2),
                ("VBUS_STAT", c_uint8, 3)]

class ChargerStatus(Union):
    _fields_ = [("val", ChargerStatusVal),
               ("asbyte", c_uint8)]

vbus_values = {
    0b000 : "No Input 001: USB Host SDP",
    0b010 : "USB CDP (1.5A)",
    0b011 : "USB DCP (3.25A)",
    0b100 : "Adjustable High Voltage DCP (MaxCharge) (1.5A)",
    0b101 : "Unknown Adapter (500mA)",
    0b110 : "Non-Standard Adapter (1A/2A/2.1A/2.4A)",
    0b111 : "OTG",
}
# set up logging
root_log = logging.getLogger()
root_log.setLevel("DEBUG")

# set up communication to the SuperPower_UPS
bus = 1
sp = SuperPower(bus, _i2c_address, _time_const, _num_retries)

# Access data
#logging.info("Current version is " + str(sp.get_version()))
#logging.info("Current primed is " + str(sp.get_primed()))
#logging.info("Current enable_bootloader is " + str(sp.get_enable_bootloader()))
#logging.info("Current timeout is " + str(sp.get_timeout()))
#logging.info("Setting timeout")
#sp.set_timeout(60)
#logging.info("Current external voltage is " + str(sp.get_ext_voltage()))
logging.info("Current battery voltage is " + str(sp.get_bat_voltage()))
logging.info("Current battery charge current is " + str(sp.get_charge_current()))
logging.info("Current vbus voltage is " + str(sp.get_vbus_voltage()))
logging.info("Current temperature is " + str(sp.get_temperature()))

ch_status = ChargerStatus()
ch_status.asbyte = sp.get_charger_status()


logging.info("VBUS status: " + vbus_values[ch_status.val.VBUS_STAT])
