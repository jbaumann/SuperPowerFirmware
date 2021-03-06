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
                              hex(register) + " correctly: " + hex(val))
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


if __name__ == '__main__':
    main(*sys.argv[1:])
