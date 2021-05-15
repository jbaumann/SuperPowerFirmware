
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

    PRIMED = 0x00
    FORCE_SHUTDOWN = 0x01
    ENABLE_BOOTLOADER = 0x02
    RTC_ASYNC_PREDIV = 0x03
    DISPLAY_TYPE = 0x04
    CHARGER_STATUS = 0x40
    CHARGER_CONTACT = 0x41
    UPS_STATE = 0x42
    TIMEOUT = 0x80
    RESTART_VOLTAGE = 0x81
    WARN_VOLTAGE = 0x82
    UPS_SHUTDOWN_VOLTAGE = 0x83
    RTC_SYNC_PREDIV = 0x84
    UPS_BAT_VOLTAGE = 0xc0
    CHARGE_CURRENT = 0xc1
    VBUS_VOLTAGE = 0xc2
    SECONDS = 0xc3
    TEMPERATURE = 0xc4
    VERSION = 0xe0
    SHOULD_SHUTDOWN = 0xe1
    WRITE_TO_EEPROM = 0xe2
    JUMP_TO_BOOTLOADER = 0xe3
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
                logging.debug("Couldn't read data from register " +                               hex(register) + " correctly.")
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

    def get_display_type(self):
        return self.get_8bit_value(self.DISPLAY_TYPE)

    def set_display_type(self, value):
        return self.set_8bit_value(self.DISPLAY_TYPE, value)

    def get_charger_status(self):
        return self.get_8bit_value(self.CHARGER_STATUS)

    def get_charger_contact(self):
        return self.get_8bit_value(self.CHARGER_CONTACT)

    def get_ups_state(self):
        return self.get_8bit_value(self.UPS_STATE)

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

    def get_ups_bat_voltage(self):
        return self.get_16bit_value(self.UPS_BAT_VOLTAGE)

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
        DAEMON_SECTION: {
            I2C_ADDRESS: '0x40',
            LOG_LEVEL: 'DEBUG',
            BUTTON_FUNCTION: 'nothing',
            SLEEPTIME: '20',
            'primed': SUPERPOWER,
            'force shutdown': SUPERPOWER,
            'enable bootloader': SUPERPOWER,
            'rtc async prediv': SUPERPOWER,
            'display type': SUPERPOWER,
            'timeout': SUPERPOWER,
            'restart voltage': SUPERPOWER,
            'warn voltage': SUPERPOWER,
            'ups shutdown voltage': SUPERPOWER,
            'rtc sync prediv': SUPERPOWER,
        }
    }

