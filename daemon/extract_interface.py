#!/usr/bin/env python3
#
# Author: Joachim Baumann

import os
import sys
import re
from typing import Tuple, Any
from argparse import ArgumentParser, Namespace

_default_i2c_register_input = '../firmware/SuperPowerFirmware/Core/Inc/i2c_register.h'
_default_task_communication_input = '../firmware/SuperPowerFirmware/Core/Inc/task_communication.h'
_default_output = "superpower_i2c.py"

offsets = {}
i2c_register = {}
function_details = {}
task_communication_offset = 0xF0
task_max_data_size = 32
tasks = []

special_config_regs = {
    "I2C_ADDRESS": '0x40',
    "LOG_LEVEL": 'DEBUG',
    "BUTTON_FUNCTION": 'nothing',
    "SLEEPTIME": '20',

    #    "PRIMED": '0',
    #    "'force shutdown'": '0',
}


def main(*args):
    # parse the commandline arguments
    args = parse_cmdline(args)

# check whether output file exists
    if os.path.exists(args.output):
        if not args.force:
            print("File '%s' exists" % args.output)
            print("Use option --force to overwrite")
            quit()

# try to open input files
    with open(args.i2c_register) as f:
        i2c_register_lines = f.readlines()

    with open(args.task_communication) as f:
        task_communication_lines = f.readlines()

    # parse input file for i2c registers
    lines_iter = iter(i2c_register_lines)
    for line in lines_iter:
        match = re.search("//\s*_EXTRACT_I2C_REGISTER_", line)
        if match:
            # First check for the task communication offset
            match = re.search(
                "TASK_COMMUNICATION\s*=\s*([0-9a-fA-FxX]+)", line)
            if match:
                task_communication_offset = int(match.group(1), 0)
                # add temporary marker to recognize register collisions
                i2c_register[task_communication_offset] = "for Task Communication"
            # Now check for offset definition
            match = re.search(
                "(CONFIG|STATUS|SPECIAL)_(8|16)BIT_OFFSET\s*=\s*([0-9a-fA-FxX]+)", line)
            if match:
                type = match.group(1).lower()
                if not type in offsets:
                    offsets[type] = {}
                offsets[type][match.group(2)] = int(match.group(3), 0)

            # Check for beginning of a struct and extract name
            match = re.search("struct\s+([^ \d\W]\w+)\s*{", line)
            if match:
                identifier = match.group(1)
                match = re.search(
                    "(Status|Config|Special).*(8|16).*", identifier)
                const_name = match.group(0)
                type = match.group(1).lower()
                size = match.group(2)

                # now extract all registers and store them
                reg_number = offsets[type][size]
                next_line = next(lines_iter)
                while not re.search("//\s*_EXTRACT_I2C_REGISTER_", next_line):
                    match = re.search("\s+([^ \d\W]\w+)\s*;", next_line)
                    if match:
                        reg_name = match.group(1)
                        if reg_number in i2c_register:
                            print(
                                "WARNING: Register collision for register %#2.2x" % reg_number)
                            print("         Register %s and %s" %
                                  (i2c_register[reg_number], reg_name))
                            print(
                                "         Fix: Adjust you settings for %s" % (const_name))
                        i2c_register[reg_number] = reg_name
                        getter = type == "config" or type == "status"
                        function_details[reg_number] = {
                            "getter": getter, "setter": type == "config", "size": size}
                        reg_number += 1
                    next_line = next(lines_iter)

    # Remove temporary marker
    i2c_register.pop(task_communication_offset, None)

    # parse input file for task communication
    lines_iter = iter(task_communication_lines)
    for line in lines_iter:
        match = re.search("//\s*_EXTRACT_TASK_COMM_", line)
        if match:
            # extract the max_data_size
            match = re.search(
                "TASK_MAX_DATA_SIZE\s*=\s*([0-9a-fA-FxX]+)", line)
            if match:
                task_max_data_size = int(match.group(1), 0)
            match = re.search(
                ".callback\s*=\s*([^\s,]+)", line)
            if match:
                callback = match.group(1).lower()
                if callback != "null":
                    # clean up the name
                    cleanup = ["callback", "_"]
                    for trailing in cleanup:
                        if callback.endswith(trailing):
                            callback = callback[:-len(trailing)]
                tasks.append(callback)

    # write output file
    file = open(args.output, "w")

    # Write the import statements and class definition to file
    print(class_preamble, file=file)

    # Write the constants to file
    for register in sorted(i2c_register):
        print("    %s = %#2.2x" %
              (i2c_register[register].upper(), register), file=file)
    task_counter = task_communication_offset
    for task in tasks:
        if task != "null":
            if task_counter > 0xFF:
                print("WARNING: You will not be able to use task %s" % task)
                print("         Register %#2.2x will not be reachable" %
                      task_counter)
                print("         Fix: Adjust your settings for TASK_COMMUNICATION")
            print("    %s = %#2.2x" %
                  (task.upper(), task_counter), file=file)
            task_counter = task_counter + 1

    # Write the internal attributes and methods to file
    print(class_functions % task_max_data_size, file=file)

    # Generate the member functions for each register.
    # For config registers a set()- and a get()-method is generated,
    # for status registers only a get()-method is generated.
    for register in sorted(i2c_register):
        name = i2c_register[register]
        if function_details[register]["getter"]:
            print(get_method %
                  (name, function_details[register]["size"], name.upper()), file=file)
        if function_details[register]["setter"]:
            print(set_method %
                  (name, function_details[register]["size"], name.upper()), file=file)

    for task in tasks:
        if task != "null":
            print(send_method % (task, task.upper()), file=file)
            print(receive_method % (task, task.upper()), file=file)

    # Now the Config class
    print(config_preamble, file=file)
    # print the special config registers
    for option in special_config_regs:
        print("            %s: '%s'," % (
            option, special_config_regs[option]), file=file)

    for register in sorted(i2c_register):
        name = i2c_register[register]
        if function_details[register]["setter"]:
            name = "'" + name.replace("_", " ") + "'"
            # we have a config register
            if name not in special_config_regs:
                print(config_entry % name, file=file)

    print(config_postfix, file=file)

    # Cleanup
    file.close()


def parse_cmdline(args: Tuple[Any]) -> Namespace:
    arg_parser = ArgumentParser(description='Extract I2C Register Definitions')
    arg_parser.add_argument('-i', '--i2c_register', metavar='file', required=False,
                            default=_default_i2c_register_input,
                            help='full path and name of the i2c_registers input file')
    arg_parser.add_argument('-t', '--task_communication', metavar='file', required=False,
                            default=_default_task_communication_input,
                            help='full path and name of the task_communication input file')
    arg_parser.add_argument('-o', '--output', metavar='file', required=False,
                            default=_default_output,
                            help='full path and name of the output file')
    arg_parser.add_argument('-f', '--force', required=False, action='store_true',
                            help='overwrite outputfile if it exists')
    return arg_parser.parse_args(args)


class_preamble = """
# Author Joachim Baumann

import logging
import os
import sys
import time
import smbus
import struct
from typing import Tuple, Any
from collections.abc import Mapping
from pathlib import Path
from configparser import ConfigParser

class SuperPower:
"""

class_functions = """
    _POLYNOME = 0x31
    _TASK_MAX_DATA_SIZE = %s

    def __init__(self, bus_number, address, time_const, num_retries):
        self._bus_number = bus_number
        self._address = address
        self._time_const = time_const
        self._num_retries = num_retries

    def addCrc(self, crc, n):
        for bitnumber in range(0,8):
            if ( n ^ crc ) & 0x80 : crc = ( crc << 1 ) ^ self._POLYNOME
            else                  : crc = ( crc << 1 )
            n = n << 1
        return crc & 0xFF

    def calcCRC(self, register, read, len):
        crc = self.addCrc(0, register)
        for elem in range(0, len):
            crc = self.addCrc(crc, read[elem])
        return crc

    def set_8bit_value(self, register, value):
        crc = self.addCrc(0, register)
        crc = self.addCrc(crc, value)

        arg_list = [value, crc]
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                bus.write_i2c_block_data(self._address, register, arg_list)
                bus.close()
                if (self.get_8bit_value(register)) == value:
                    return True
            except Exception as e:
                logging.debug("Couldn't set 8 bit register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning("Couldn't set 8 bit register after " + \
                        str(x) + " retries.")
        return False

    def get_8bit_value(self, register):
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                read = bus.read_i2c_block_data(self._address, register, 2)
                val = read[0]
                bus.close()
                if read[1] == self.calcCRC(register, read, 1):
                    return val
                logging.debug("Couldn't read register " + \
                              hex(register) + " correctly: " + hex(val))
            except Exception as e:
                logging.debug("Couldn't read 8 bit register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't read 8 bit register after " + str(x) + " retries.")
        return 0xFFFF

    def set_16bit_value(self, register, value):
        # we interpret every value as a 16-bit signed value
        vals = value.to_bytes(2, byteorder='little', signed=True)
        crc = self.calcCRC(register, vals, 2)

        arg_list = [vals[0], vals[1], crc]

        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                bus.write_i2c_block_data(self._address, register, arg_list)
                bus.close()
                if (self.get_16bit_value(register)) == value:
                    return True
            except Exception as e:
                logging.debug("Couldn't set 16 bit register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't set 16 bit register after " + str(x) + " retries.")
        return False

    def get_16bit_value(self, register):
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                read = bus.read_i2c_block_data(self._address, register, 3)
                # we interpret every value as a 16-bit signed value
                val = int.from_bytes(
                    read[0:2], byteorder='little', signed=True)
                bus.close()
                if read[2] == self.calcCRC(register, read, 2):
                    return val
                logging.debug("Couldn't read 16 bit register " + \
                              hex(register) + " correctly.")
            except Exception as e:
                logging.debug("Couldn't read 16 bit register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't read 16 bit register after " + str(x) + " retries.")
        return 0xFFFFFFFF

    def get_version(self):
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                read = bus.read_i2c_block_data(self._address, self.VERSION, 4)
                bus.close()
                if read[3] == self.calcCRC(self.VERSION, read, 3):
                    major = read[2]
                    minor = read[1]
                    patch = read[0]
                    return (major, minor, patch)
                logging.debug("Couldn't read version information correctly.")
            except Exception as e:
                logging.debug(
                    "Couldn't read version information. Exception: " + str(e))
        logging.warning(
            "Couldn't read version information after " + str(x) + " retries.")
        return (0xFFFF, 0xFFFF, 0xFFFF)

    def jump_to_bootloader(self):
        register = self.JUMP_TO_BOOTLOADER
        value = 1

        crc = self.addCrc(0, register)
        crc = self.addCrc(crc, value)

        arg_list = [value, crc]
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                bus.write_i2c_block_data(self._address, register, arg_list)
                bus.close()
                return True
            except Exception as e:
                logging.debug("Couldn't jump to bootloader " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning("Couldn't jump to bootloader after " + \
                        str(x) + " retries.")
        return False

    def get_should_shutdown(self):
        return self.get_8bit_value(self.SHOULD_SHUTDOWN)

    def set_should_shutdown(self, value):
        return self.set_8bit_value(self.SHOULD_SHUTDOWN, value)

    def get_uptime(self):
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                read = bus.read_i2c_block_data(
                    self._address, self.REG_UPTIME, 5)
                bus.close()
                if read[4] == self.calcCRC(self.REG_UPTIME, read, 4):
                    uptime = int.from_bytes(
                        read[0:3], byteorder='little', signed=False)
                    return uptime
                logging.debug("Couldn't read uptime information correctly.")
            except Exception as e:
                logging.debug(
                    "Couldn't read uptime information. Exception: " + str(e))
        logging.warning(
            "Couldn't read uptime information after " + str(x) + " retries.")
        return 0xFFFFFFFFFFFF

    def send_to_task(self, register, values):
        if len(values) > self._TASK_MAX_DATA_SIZE:
            return False
        crc = self.addCrc(0, register)
        crc = self.calcCRC(register, values, len(values))

        values.append(crc)
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                bus.write_i2c_block_data(self._address, register, values)
                bus.close()
                return True
            except Exception as e:
                logging.debug("Couldn't send data to register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't send data to register after " + str(x) + " retries.")
        return False

    def receive_from_task(self, register, num_bytes):
        if num_bytes > self._TASK_MAX_DATA_SIZE:
            num_bytes = self._TASK_MAX_DATA_SIZE
        for x in range(self._num_retries):
            bus = smbus.SMBus(self._bus_number)
            time.sleep(self._time_const)
            try:
                read = bus.read_i2c_block_data(
                    self._address, register, num_bytes + 1)
                bus.close()
                if read[num_bytes] == self.calcCRC(register, read, num_bytes):
                    read.pop()
                    return read
                logging.debug("Couldn't read data from register " + \
                              hex(register) + " correctly.")
            except Exception as e:
                logging.debug("Couldn't read data from register " + \
                              hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't read 8 bit register after " + str(x) + " retries.")
        return 0xFFFF
"""

get_method = """
    def get_%s(self):
        return self.get_%sbit_value(self.%s)"""

set_method = """
    def set_%s(self, value):
        return self.set_%sbit_value(self.%s, value)"""

send_method = """
    def task_send_to_%s(self, values):
        return self.send_to_task(self.%s, values)"""

receive_method = """
    def task_receive_from_%s(self, num_bytes):
        return self.receive_from_task(self.%s, num_bytes)"""


config_preamble = """
class SuperPowerConfig:
    DAEMON_SECTION = "superpower"
    SUPERPOWER = 'SUPERPOWER'
    I2C_ADDRESS = 'i2c address'
    LOG_LEVEL = 'loglevel'
    BUTTON_FUNCTION = 'button function'
    SLEEPTIME = "sleep time"
    PRIMED = "primed"

    def __init__(self, cfgfile=None):
        self.parser = ConfigParser(allow_no_value=True)
        if cfgfile:
            self.configfile_name = cfgfile
        else:
            self.configfile_name = str(Path(__file__).parent.absolute()) + "/superpower_daemon.cfg"

        self.default_parser = ConfigParser(allow_no_value=True)
        self.default_parser.read_dict(self.DEFAULT_CONFIG)
        self.read_config()

    def get(self, entry):
        if entry in self.parser[self.DAEMON_SECTION]:
            return self.parser[self.DAEMON_SECTION][entry]
        else:
            return self.default_parser[self.DAEMON_SECTION][entry]

    def read_config(self):
        if not os.path.isfile(self.configfile_name):
            logging.info("No Config File. Trying to create one.")
            self.parser.add_section(self.DAEMON_SECTION)
        else:
            try:
                self.parser.read(self.configfile_name)
                logging.debug("Config has been read")
            except Exception:
                logging.info("cannot read config file. Using default values")

    def write_config(self):
        try:
            cfgfile = open(self.configfile_name, 'w')
            self.parser.write(cfgfile)
            cfgfile.close()
        except Exception:
            logging.warning("cannot write config file.")

    def validate_config(self):
        # TODO generate the keys instead of hardcoding them
        WARN_VOLTAGE = 'warn voltage'
        SHUTDOWN_VOLTAGE = 'ups shutdown voltage'
        RESTART_VOLTAGE = 'restart voltage'
        low_val = 3000
        high_val = 4200
        set_to_default = False
        wv = -1
        sv = -1
        rv = -1

        # First check the absolute values
        try:
            wv = int(self.get(WARN_VOLTAGE))
            if wv < low_val or wv > high_val:
                logging.warning("Config: Warn voltage outside limits. Setting all voltages to default")
                set_to_default = True
        except ValueError:
            # Voltage is not set
            pass
        except KeyError:
            # The key is not valid, we have a problem
            logging.error("Key %s is not valid. Cannot validate the value." % WARN_VOLTAGE)

        try:
            sv = int(self.get(SHUTDOWN_VOLTAGE))
            if sv < low_val or sv > high_val:
                logging.warning("Config: Shutdown voltage outside limits. Setting all voltages to default")
                set_to_default = True
        except ValueError:
            # Voltage is not set
            pass
        except KeyError:
            # The key is not valid, we have a problem
            logging.error("Key %s is not valid. Cannot validate the value." % SHUTDOWN_VOLTAGE)

        try:
            rv = int(self.get(RESTART_VOLTAGE))
            if rv < low_val or rv > high_val:
                logging.warning("Config: Restart voltage outside limits. Setting all voltages to default")
                set_to_default = True
        except ValueError:
            # Voltage is not set
            pass
        except KeyError:
            # The key is not valid, we have a problem
            logging.error("Key %s is not valid. Cannot validate the value." % RESTART_VOLTAGE)

        if set_to_default == True:
            if wv != -1:
                self.parser[self.DAEMON_SECTION].pop(WARN_VOLTAGE, None)
            if sv != -1:
                self.parser[self.DAEMON_SECTION].pop(SHUTDOWN_VOLTAGE, None)
            if rv != -1:
                self.parser[self.DAEMON_SECTION].pop(RESTART_VOLTAGE, None)
            return

        if wv == -1 or sv == -1 or rv == -1:
            # we had a key error and cannot do anything further
            return
        # Check the relation of values
        # We know that all voltages are inside the limits
        voltage_changed = False
        min_distance = 200
        if wv < sv:
            logging.warning("Config: Warn voltage < shutdown voltage. Taking corrective action.")
            sv = wv - min_distance
            voltage_changed = True
        if wv < sv or wv > rv:
            logging.warning("Config: Warn voltage > restart voltage. Taking corrective action.")
            rv = wv + min_distance
            voltage_changed = True
        if sv < low_val:
            diff = low_val - sv
            sv = low_val
            wv = wv + diff
            if rv - wv < min_distance:
                rv = wv + min_distance
            voltage_changed = True
        if rv > high_val:
            diff = rv - high_val
            rv = high_val
            wv = wv - diff
            if wv - sv < min_distance:
                sv = wv - min_distance
            voltage_changed = True

        if voltage_changed == True:
            # Write the new values back to the config
            self.parser[self.DAEMON_SECTION][WARN_VOLTAGE] = wv
            self.parser[self.DAEMON_SECTION][SHUTDOWN_VOLTAGE] = sv
            self.parser[self.DAEMON_SECTION][RESTART_VOLTAGE] = rv
            logging.warning("Corrected voltage values have been written to the config")
            return

        logging.info("Config validated")

    def merge_and_sync_values(self, superpower):
        changed_config = False
        logging.debug("Merging")

        default_daemon_config = self.default_parser[self.DAEMON_SECTION]
        daemon_config = self.parser[self.DAEMON_SECTION]
        for option in default_daemon_config:
            if option not in daemon_config:
                if default_daemon_config[option] == self.SUPERPOWER:
                    # get data from superpower
                    logging.debug("Fetching %s" % option)
                    # create correct method name and call it
                    method_name = "get_" + option.replace(" ", "_")
                    method_ref = getattr(superpower, method_name)
                    result = method_ref()
                    daemon_config[option] = str(result)
                else:
                    daemon_config[option] = default_daemon_config[option]
                    logging.debug("Set Config value %s : %s" % (option, daemon_config[option]))
                changed_config = True
            else:
                if default_daemon_config[option] == self.SUPERPOWER:
                    # get data from superpower
                    logging.debug("Fetching %s" % option)
                    # create correct method name and call it
                    method_name = "get_" + option.replace(" ", "_")
                    method_ref = getattr(superpower, method_name)
                    result = method_ref()
                    if result != int(daemon_config[option]):
                        # send data to Superpower
                        logging.debug("Set SuperPower value: %s" % option)
                        # create correct method name and call it
                        method_name = "set_" + option.replace(" ", "_")
                        method_ref = getattr(superpower, method_name)
                        result = method_ref(int(daemon_config[option]))

        if changed_config:
            logging.info("Writing new config file")
            self.write_config()

    DEFAULT_CONFIG = {
        DAEMON_SECTION: {"""

config_entry = "            %s: SUPERPOWER,"

config_postfix = """        }
    }
"""

if __name__ == '__main__':
    main(*sys.argv[1:])
