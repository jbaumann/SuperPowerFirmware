#!/usr/bin/env python3

import sys

#sys.path.append('/opt/superpowerdaemon/')  # add the path to our SuperPower module

import time
import logging
from superpower_i2c import SuperPower

_time_const  = 0.1  # used as a pause between i2c communications
_num_retries = 1   # the number of retries when reading from or writing to the SuperPower UPS
_i2c_address = 0x40 # the I2C address that is used for the SuperPower UPS

# set up logging
root_log = logging.getLogger()
root_log.setLevel("DEBUG")

# set up communication to the SuperPower_UPS
bus = 1
sp = SuperPower(bus, _i2c_address, _time_const, _num_retries)

# access data
#logging.info("Current primed is " + str(sp.get_primed()))
#logging.info("Current enable_bootloader is " + str(sp.get_enable_bootloader()))
#logging.info("Current timeout is " + str(sp.get_timeout()))
logging.info("Setting timeout")
sp.set_timeout(60)
