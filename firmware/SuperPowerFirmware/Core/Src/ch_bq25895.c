/*
 * ch_bq25895.c
 *
 *  Created on: Dec 26, 2020
 *      Author: jbaumann
 *
 *  This file contains functions to handle the BQ25895 charger IC.
 *  This includes helper function to init the chip and to send data
 *  to it, and conversion functions that calculate values from the
 *  raw bits returned by the chip according to its datasheet.
 */

#include "main.h"
#include "ch_bq25895.h"

I2C_CH_BQ25895_Register i2c_ch_BQ25895_register;
uint8_t *i2c_ch_BQ25895_register_reg = (uint8_t *) &i2c_ch_BQ25895_register;

/*
 * Initialize the BQ25895 charger chip
 */
HAL_StatusTypeDef ch_init(I2C_HandleTypeDef *hi2c) {
	HAL_StatusTypeDef ret_val;

	// stop watchdog timer
	ret_val = ch_transfer_byte_to_register(hi2c, CH_WATCHDOG, CH_WATCHDOG_STOP);
	// 3.25A input current limit, ILIM pin disabled
	ret_val = ch_transfer_byte_to_register(hi2c, CH_ILIM, CH_ILIM_MAX);
	// set SYS_MIN to 3.0V
	ret_val = ch_transfer_byte_to_register(hi2c, CH_CONFIG, CH_CONFIG_SYS_MIN);

	return ret_val;
}

/*
 * Convenience method that writes one byte of data to the i2c_register
 * of the charger using the predefined charger address
 */
HAL_StatusTypeDef ch_transfer_byte_to_register(I2C_HandleTypeDef *hi2c, uint8_t i2c_register, uint8_t data) {
	uint8_t ch_buf[2];
	ch_buf[0] = i2c_register;
	ch_buf[1] = data;
	return HAL_I2C_Master_Transmit(hi2c, CHARGER_ADDRESS, ch_buf, 2, ch_i2c_master_timeout);
}


/*
 * Table 22. REG0E
 * ADC conversion of Battery Voltage (VBAT)
 * Offset: 2.304V
 * Range: 2.304V (0000000) – 4.848V (1111111)
 * Default: 2.304V (0000000)
 *
 * BATV[6] 1280mV
 * BATV[5]  640mV
 * BATV[4]  320mV
 * BATV[3]  160mV
 * BATV[2]   80mV
 * BATV[1]   40mV
 * BATV[0]   20mV
 */
uint16_t ch_convert_batv(uint8_t raw) {
	uint16_t result = 2304;
	uint16_t bit_val = 20;
	for(int i = 0; i < 7; i++) {
		if(raw & 0x1) {
			result += bit_val;
		}
		bit_val <<= 1;
		raw >>= 1;
	}
	return result;
}


/*
 * Table 25 REG11
 * ADC conversion of VBUS voltage (VBUS)
 * Offset: 2.6V
 * Default: 2.6V (0000000)
 *
 * VBUS[7] VBUS_GD - 0 no VBUS - 1 VBUS attached
 * VBUSV[6] 6400mV
 * VBUSV[5] 3200mV
 * VBUSV[4] 1600mV
 * VBUSV[3]  800mV
 * VBUSV[2]  400mV
 * VBUSV[1]  200mV
 * VBUSV[0]  100mV
 */

uint16_t ch_convert_vbus(uint8_t raw) {
	uint16_t result = 2600;
	if(raw & 0b10000000) { // VBUS good
		uint16_t bit_val = 100;
		for(int i = 0; i < 7; i++) {
			if(raw & 0x1) {
				result += bit_val;
			}
			bit_val <<= 1;
			raw >>= 1;
		}
		return result;
	}
	return 0;
}

/*
 * Table 26 REG12
 * ADC conversion of Charge Current (IBAT) when VBAT > VBATSHORT
 * Offset: 0mA
 * Range 0mA (0000000) – 6350mA (1111111) Default: 0mA (0000000)
 * Note: This register returns 0000000 for VBAT < VBATSHORT
 * ICHGR[6] 3200mA
 * ICHGR[5] 1600mA
 * ICHGR[4]  800mA
 * ICHGR[3]  400mA
 * ICHGR[2]  200mA
 * ICHGR[1]  100mA
 * ICHGR[0]   50mA
 */

uint16_t ch_convert_charge_current(uint8_t raw) {
	uint16_t result = 0;
	uint16_t bit_val = 50;
	for(int i = 0; i < 7; i++) {
		if(raw & 0x1) {
			result += bit_val;
		}
		bit_val <<= 1;
		raw >>= 1;
	}

	return result;
}
