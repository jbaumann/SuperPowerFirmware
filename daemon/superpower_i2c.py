
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

    PRIMED = 0x00
    FORCE_SHUTDOWN = 0x01
    ENABLE_BOOTLOADER = 0x02
    RTC_ASYNC_PREDIV = 0x03
    SHOULD_SHUTDOWN = 0x40
    CHARGER_STATUS = 0x41
    CHARGER_CONTACT = 0x42
    TIMEOUT = 0x80
    RESTART_VOLTAGE = 0x81
    WARN_VOLTAGE = 0x82
    UPS_SHUTDOWN_VOLTAGE = 0x83
    RTC_SYNC_PREDIV = 0x84
    BAT_VOLTAGE = 0xc0
    CHARGE_CURRENT = 0xc1
    VBUS_VOLTAGE = 0xc2
    SECONDS = 0xc3
    TEMPERATURE = 0xc4
    VERSION = 0xe0
    WRITE_TO_EEPROM = 0xe1
    JUMP_TO_BOOTLOADER = 0xe2
    TEST = 0xf0

    _POLYNOME = 0x31
    _TASK_MAX_DATA_SIZE = 32

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
                logging.debug("Couldn't set 8 bit register " +                               hex(register) + ". Exception: " + str(e))
        logging.warning("Couldn't set 8 bit register after " +                         str(x) + " retries.")
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
                logging.debug("Couldn't read register " +                               hex(register) + " correctly: " + hex(val))
            except Exception as e:
                logging.debug("Couldn't read 8 bit register " +                               hex(register) + ". Exception: " + str(e))
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
                logging.debug("Couldn't set 16 bit register " +                               hex(register) + ". Exception: " + str(e))
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
                logging.debug("Couldn't read 16 bit register " +                               hex(register) + " correctly.")
            except Exception as e:
                logging.debug("Couldn't read 16 bit register " +                               hex(register) + ". Exception: " + str(e))
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
                logging.debug("Couldn't jump to bootloader " +                               hex(register) + ". Exception: " + str(e))
        logging.warning("Couldn't jump to bootloader after " +                         str(x) + " retries.")
        return False


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
                logging.debug("Couldn't send data to register " +                               hex(register) + ". Exception: " + str(e))
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
                logging.debug("Couldn't read data from register " +                               hex(register) + " correctly: " + hex(val))
            except Exception as e:
                logging.debug("Couldn't read data from register " +                               hex(register) + ". Exception: " + str(e))
        logging.warning(
            "Couldn't read 8 bit register after " + str(x) + " retries.")
        return 0xFFFF


    def get_primed(self):
        return self.get_8bit_value(self.PRIMED)

    def set_primed(self, value):
        return self.set_8bit_value(self.PRIMED, value)

    def get_force_shutdown(self):
        return self.get_8bit_value(self.FORCE_SHUTDOWN)

    def set_force_shutdown(self, value):
        return self.set_8bit_value(self.FORCE_SHUTDOWN, value)

    def get_enable_bootloader(self):
        return self.get_8bit_value(self.ENABLE_BOOTLOADER)

    def set_enable_bootloader(self, value):
        return self.set_8bit_value(self.ENABLE_BOOTLOADER, value)

    def get_rtc_async_prediv(self):
        return self.get_8bit_value(self.RTC_ASYNC_PREDIV)

    def set_rtc_async_prediv(self, value):
        return self.set_8bit_value(self.RTC_ASYNC_PREDIV, value)

    def get_should_shutdown(self):
        return self.get_8bit_value(self.SHOULD_SHUTDOWN)

    def get_charger_status(self):
        return self.get_8bit_value(self.CHARGER_STATUS)

    def get_charger_contact(self):
        return self.get_8bit_value(self.CHARGER_CONTACT)

    def get_timeout(self):
        return self.get_16bit_value(self.TIMEOUT)

    def set_timeout(self, value):
        return self.set_16bit_value(self.TIMEOUT, value)

    def get_restart_voltage(self):
        return self.get_16bit_value(self.RESTART_VOLTAGE)

    def set_restart_voltage(self, value):
        return self.set_16bit_value(self.RESTART_VOLTAGE, value)

    def get_warn_voltage(self):
        return self.get_16bit_value(self.WARN_VOLTAGE)

    def set_warn_voltage(self, value):
        return self.set_16bit_value(self.WARN_VOLTAGE, value)

    def get_ups_shutdown_voltage(self):
        return self.get_16bit_value(self.UPS_SHUTDOWN_VOLTAGE)

    def set_ups_shutdown_voltage(self, value):
        return self.set_16bit_value(self.UPS_SHUTDOWN_VOLTAGE, value)

    def get_rtc_sync_prediv(self):
        return self.get_16bit_value(self.RTC_SYNC_PREDIV)

    def set_rtc_sync_prediv(self, value):
        return self.set_16bit_value(self.RTC_SYNC_PREDIV, value)

    def get_bat_voltage(self):
        return self.get_16bit_value(self.BAT_VOLTAGE)

    def get_charge_current(self):
        return self.get_16bit_value(self.CHARGE_CURRENT)

    def get_vbus_voltage(self):
        return self.get_16bit_value(self.VBUS_VOLTAGE)

    def get_seconds(self):
        return self.get_16bit_value(self.SECONDS)

    def get_temperature(self):
        return self.get_16bit_value(self.TEMPERATURE)

    def task_send_to_test(self, values):
        return self.send_to_task(self.TEST, values)

    def task_receive_from_test(self, num_bytes):
        return self.receive_from_task(self.TEST, num_bytes)
