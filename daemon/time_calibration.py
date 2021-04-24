import statistics
from time import gmtime, strftime, sleep
from datetime import datetime, timedelta
import numpy as np
from statistics import StatisticsError, mode
from superpower_i2c import SuperPower
from smbus import SMBus


class RTCCalibration:

    RTC_ASYNC_PREDIV_REGISTER = 10

    def __init__(self, bus_number=1, address=0x41):
        self.RTC_ID = 0xF2
        self.address = address
        self.i2c_bus = bus_number
        self.initial_error = 0
        self.min_percentage = 50
        self.rtc_async_register = 249
        self.rtc_subfs = 0
        self.bus = SMBus(self.i2c_bus)


    def hex_to_bcd(self, value):
        msb = int(value/10) << 4
        lsb = value % 10
        return msb + lsb


    def bcd_to_int(self, value):
        msb = ((value & 0xF0) >> 4) * 10
        lsb = value & 0x0F
        return msb + lsb

    def send_to_mc(self, data):
        register = data.to_bytes(2,'big')
        clase = SuperPower(1,0x41,0.1,1)
        clase.send_to_task(self.RTC_ID,[10, register[0], register[1]])

    def setCurrentTime(self):
        next_interval = datetime.now() + timedelta(seconds=1)
        while datetime.now() < next_interval:
            pass
        utc_time = datetime.utcnow()
        data_block = [self.hex_to_bcd(utc_time.second), self.hex_to_bcd(utc_time.minute), self.hex_to_bcd(utc_time.hour)]
        self.bus.write_i2c_block_data(self.address, 0, data_block)
        return utc_time.strftime("%H:%M:%S.%f")


    def get_error(self, bus_id=1, address=0x41, verbose=False):
        bus = SMBus(bus_id)
        rtc_time = bus.read_i2c_block_data(address,0,3)
        system_time = datetime.utcnow()
        system_seconds = system_time.second + (system_time.minute * 60) + (system_time.hour * 60 * 60) + float('0.'+str(system_time.microsecond))
        rtc_seconds = self.bcd_to_int(rtc_time[0]) + (self.bcd_to_int(rtc_time[1]) * 60) + (self.bcd_to_int(rtc_time[2]) * 60 * 60)
        error = round(rtc_seconds - system_seconds, 3)
        timedelta(hours=self.bcd_to_int(rtc_time[2]), minutes=self.bcd_to_int(rtc_time[1]), seconds=self.bcd_to_int(rtc_time[0]))
        if verbose:
            print('RTC time:    {:02d}:{:02d}:{:02d}'.format(self.bcd_to_int(rtc_time[2]), self.bcd_to_int(rtc_time[1]), self.bcd_to_int(rtc_time[0])))
            print('System Time: ' + datetime.utcnow().strftime('%H:%M:%S.%f')[:-3])
            if error > 0:
                print('rtc ahead by: {:.3f} seconds'.format(abs(error)))
            else:
                print('rtc behind by: {:.3f} seconds'.format(abs(error)))
        return error

    def collect_history(self, interval, duration, verbose=False):
        self.initial_error = self.get_error(self.i2c_bus, self.address)
        history = [0]
        delta = []
        current_delta = 0
        next_interval = datetime.now() + timedelta(seconds=interval)
        amount_of_time = datetime.now() + timedelta(minutes=duration)
        try:
            while datetime.now() < amount_of_time:
                while datetime.now() < next_interval:
                    sleep(0.1)
                current_error = self.get_error(self.i2c_bus,self.address)
                history.append(current_error)
                current_delta = round(history[-1] - history[-2], 3)
                delta.append(current_delta)
                if len(history):
                    print('delta: {}'.format(current_delta))
                next_interval = datetime.now() + timedelta(seconds=interval)
                hour_error = abs(history[-1] * 2 * 60)
                error = str(timedelta(seconds=hour_error))
                #print('ERROR: ' + error)
        except Exception as e:
            print(e)
        finally:
            return history, delta

    def print_histogram(self, data):
        hist, bin_edges= np.histogram(data, density=True)
        percentages = hist * np.diff(bin_edges) * 100
        max = percentages.max()
        bar = '----------'
        error = None
        for i, percentage in enumerate(percentages):
            scale = int(round((percentage * len(bar)) / 100))
            left_range = bin_edges[i]
            right_range = bin_edges[i+1]
            left_r = '[' if i > 0 else '['
            right_r = ')' if i < len(bar) -1 else ']'
            if percentage == max:
                error = (bin_edges[i] + bin_edges[i+1]) / 2
            print('{} {:.1f}% \t{}{:.2f}, {:.2f}{}'.format(
                bar.replace('-', '#', scale),
                percentage,
                left_r,
                left_range,
                right_range,
                right_r)
                  )
        return error

    def rtc_measurement(self,interval=30,duration=10):
        print("set current time " + self.setCurrentTime())
        print('taking samples')
        error = 0
        history, delta = self.collect_history(interval, duration)
        self.print_histogram(delta)
        bins = len(np.unique(delta))
        hist, bin_edges = np.histogram(delta, bins=bins, density=True)
        percentages = hist * np.diff(bin_edges) * 100
        try:
            error = mode(delta)
        except statistics.StatisticsError:
            error = np.median(delta)
        median = np.median(delta)
        print('el delta mas repetido es: {} con porcentaje: {}'.format(error, percentages.max()))
        if percentages.max() >= self.min_percentage:
            return error, False
        print('regresando median por que fue menor')
        return median,  True


    def fine_calibration(self, coarse_error):
        if coarse_error > 0:
            self.rtc_subfs = int(coarse_error * (self.rtc_async_register + 1))
        elif coarse_error < 0:
            self.rtc_subfs = int((coarse_error - 1) * (self.rtc_async_register + 1) * (-1))
        return self.rtc_subfs


    def calibrate(self):
        initial_error,  valid = self.rtc_measurement()
        print('desired frequency = 1Hz')
        freq = 1 + initial_error
        print('actual frequency = {}'.format(freq))

        if initial_error < 0.05 and initial_error > -0.06:
            print('no corse calibration required')
            return initial_error

        register = self.rtc_async_register + int(25 * (initial_error/abs(initial_error)))
        register = register.to_bytes(2,'big')
        clase = SuperPower(1,0x41,0.1,1)
        clase.send_to_task(0xf1,[10, register[0], register[1]])

        error, valid = self.rtc_measurement()
        delta = abs(initial_error - error)
        step = delta/25
        increment = initial_error/step
        self.rtc_async_register = self.rtc_async_register + (int(round(increment,3)))
        print('calculated value for register: {}'.format(self.rtc_async_register))

        register = self.rtc_async_register
        register = register.to_bytes(2,'big')
        clase.send_to_task(0xf1,[10, register[0], register[1]])
        print('fine calibration')
        error_validation, valid = self.rtc_measurement(interval=60)

        delta = abs(error - error_validation)
        step = delta/60
        increment = error_validation/delta
        self.rtc_async_register = self.rtc_async_register + int(round(increment, 3))
        print('calculated value for register: {}'.format(self.rtc_async_register))
        register = self.rtc_async_register
        register = register.to_bytes(2,'big')
        clase.send_to_task(0xf1,[10, register[0], register[1]])

        freq = (1 + error_validation) * (self.rtc_async_register+1) * (127 + 1)
        print('The freq of the LSI is: {}'.format(freq))
        return initial_error, error, delta, freq, error_validation
