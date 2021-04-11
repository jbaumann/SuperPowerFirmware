#!/usr/bin/python3 -u

# Author Joachim Baumann

import logging
import os
import sys
import time
import struct
from typing import Tuple, Any
from argparse import ArgumentParser, Namespace
# from pathlib import Path
from superpower_i2c import SuperPower, SuperPowerConfig

# Global configuration of the daemon. You should know what you do if you change
# these values.

# Version information
major = 1
minor = 0
patch = 7

# sudo allows us to start as user 'pi'
_shutdown_cmd = "sudo systemctl poweroff"
# sudo allows us to start as user 'pi'
_reboot_cmd = "sudo systemctl reboot"
_time_const = 0.1  # used as a pause between i2c communications
_num_retries = 10  # the number of retries when reading from or writing to the UPS

# These are the different values reported back by the UPS depending on its config
button_level = 2**3
SL_INITIATED = 2  # the value we use to signal that we are shutting down
shutdown_levels = {
    # 0: Normal mode
    0: "Everything is normal.",
    # 2 is reserved for us signalling the UPS that we are shutting down
    # 4-15: Maybe shutdown or restart, depending on configuration
    2**2: "No external voltage detected. We are on battery power.",
    button_level: "Button has been pressed. Reacting according to configuration.",
    # >16: Definitely shut down
    2**7: "Battery is at warn level. Shutting down.",
}
# Here we store the button functions that are called depending on the configuration
button_functions = {
    "nothing": lambda: logging.info("Button pressed. Configured to do nothing."),
    "shutdown": lambda: os.system(_shutdown_cmd),
    "reboot": lambda: os.system(_reboot_cmd)
}

# this is the minimum reboot time we assume the RPi needs, used for a warning message
minimum_boot_time = 30

# Code starts here.
# Here be dragons...


def main(*args):
    # Startup of the daemon

    args = parse_cmdline(args)
    setup_logger(args.nodaemon)

    config = SuperPowerConfig(args.cfgfile)

    logging.info("SuperPower Daemon version " + str(major) +
                 "." + str(minor) + "." + str(patch))

    address = int(config.get(SuperPowerConfig.I2C_ADDRESS), 0)

    superpower = SuperPower(1, address, _time_const, _num_retries)

    (sp_major, sp_minor, sp_patch) = superpower.get_version()

    if sp_major == 0xFFFF:
        logging.error("Cannot access SuperPower")
        exit(1)

    logging.info("SuperPower firmware version " + str(sp_major) +
                 "." + str(sp_minor) + "." + str(sp_patch))

    if major != sp_major:
        logging.error(
            "Daemon and Firmware major version mismatch. This might lead to serious problems. Check both versions.")

    config.merge_and_sync_values(superpower)

    logging.info("Merging completed")

    # loop until stopped or error
    fast_exit = False
    set_unprimed = False
    try:
        sleeptime = int(config.get(SuperPowerConfig.SLEEPTIME))
        while True:
            should_shutdown = superpower.get_should_shutdown()
            if should_shutdown == 0xFFFF:
                # We have a big problem
                logging.error("Lost connection to Superpower.")
                # disable to fasten restart
                # set_unprimed = True        # we still try to reset primed
                fast_exit = True
                # executes finally clause and lets the system restart the daemon
                exit(1)

            if should_shutdown > SL_INITIATED:
                # we will not exit the process but wait for the systemd to shut us down
                # using SIGTERM. This does not execute the finally clause and leaves
                # everything as it is currently configured
                global shutdown_levels
                fallback = "Unknown shutdown_level " + \
                    str(should_shutdown) + ". Shutting down."
                logging.warning(shutdown_levels.get(should_shutdown, fallback))

                if should_shutdown > 16:
                    superpower.set_should_shutdown(
                        SL_INITIATED)  # we are shutting down
                    logging.info("shutting down now...")
                    os.system(_shutdown_cmd)
                elif (should_shutdown | button_level) != 0:
                    # we are executing the button command and setting the level to normal
                    superpower.set_should_shutdown(0)
                    button_functions[config.get(SuperPowerConfig.BUTTON_FUNCTION)]()

            logging.debug("Sleeping for " +
                          config.get(SuperPowerConfig.SLEEPTIME) + " seconds.")
            time.sleep(sleeptime)

    except KeyboardInterrupt:
        logging.info("Terminating daemon: cleaning up and exiting")
        # Ctrl-C means we do not run as daemon
        set_unprimed = True
    except Exception as e:
        logging.error("An exception occurred: '" + str(e) + "' Exiting...")
    finally:
        if fast_exit == False:
            # will not be executed on SIGTERM, leaving primed set to the config value
            primed = int(config.get(SuperPowerConfig.PRIMED))
            if args.nodaemon or set_unprimed:
                primed = False
            if primed == False:
                logging.info("Trying to reset primed flag")
                superpower.set_primed(primed)
            del superpower


def parse_cmdline(args: Tuple[Any]) -> Namespace:
    arg_parser = ArgumentParser(description='SuperPower Daemon')
    arg_parser.add_argument('--cfgfile', metavar='file', required=False,
                            help='full path and name of the configfile')
    arg_parser.add_argument('--nodaemon', required=False, action='store_true',
                            help='use normal output formatting')
    return arg_parser.parse_args(args)


def setup_logger(nodaemon: bool) -> None:
    root_log = logging.getLogger()
    root_log.setLevel("DEBUG")
    if not nodaemon:
        root_log.addHandler(SystemdHandler())


class SystemdHandler(logging.Handler):
    # http://0pointer.de/public/systemd-man/sd-daemon.html
    PREFIX = {
        # EMERG <0>
        # ALERT <1>
        logging.CRITICAL: "<2>",
        logging.ERROR: "<3>",
        logging.WARNING: "<4>",
        # NOTICE <5>
        logging.INFO: "<6>",
        logging.DEBUG: "<7>",
        logging.NOTSET: "<7>"
    }

    def __init__(self, stream=sys.stdout):
        self.stream = stream
        logging.Handler.__init__(self)

    def emit(self, record):
        try:
            msg = self.PREFIX[record.levelno] + self.format(record)
            msg = msg.replace("\n", "\\n")
            self.stream.write(msg + "\n")
            self.stream.flush()
        except Exception:
            self.handleError(record)


if __name__ == '__main__':
    main(*sys.argv[1:])
