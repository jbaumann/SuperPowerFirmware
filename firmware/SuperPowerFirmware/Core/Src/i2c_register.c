/*
 * i2c_register.c
 *
 *  Created on: 3 Jan 2021
 *      Author: jbaumann
 *
 * This file contains the definition of the config registers and the initial values for
 * their contents.
 */

#include"main.h"
#include "i2c_register.h"

Config_Registers config_registers;

I2C_Config_Register_8Bit  *i2c_config_register_8bit = &(config_registers.i2c_config_register_8bit);
uint8_t  *i2c_config_register_8bit_reg = (uint8_t *) &(config_registers.i2c_config_register_8bit);

I2C_Config_Register_16Bit *i2c_config_register_16bit = &(config_registers.i2c_config_register_16bit);
uint16_t *i2c_config_register_16bit_reg = (uint16_t *) &(config_registers.i2c_config_register_16bit);

I2C_Status_Register_8Bit  _status_register_8bit, *i2c_status_register_8bit = &_status_register_8bit;
uint8_t  *i2c_status_register_8bit_reg = (uint8_t *) &_status_register_8bit;

I2C_Status_Register_16Bit _status_register_16bit, *i2c_status_register_16bit = &_status_register_16bit;
uint16_t *i2c_status_register_16bit_reg  = (uint16_t *) &_status_register_16bit;

/*
 * Initialization of the register structures
 */


Config_Registers config_registers = {
		// ID, derived from the version number
		.version = BACKUP_INIT_VALUE,
		// 8 Bit config registers
		.i2c_config_register_8bit.primed                  =    0,   // 1 if the uC should control the system
		.i2c_config_register_8bit.force_shutdown          =    0,   // 1 if the uC should shutdown the UPS if the voltage is too low (hard shutdown)
		.i2c_config_register_8bit.enable_bootloader       =    0,   // 1 if the bootloader functionality is enabled
		.i2c_config_register_8bit.rtc_async_prediv        =    0,   // the rtc async predivider that can be used to fine tune the rtc
		.i2c_config_register_8bit.display_type            =    0,   // if !=0 selects type and orientation of the display used

		// 16 Bit config registers
		.i2c_config_register_16bit.timeout                =  120,   // the timeout for the reset, should cover shutdown and reboot
		.i2c_config_register_16bit.restart_voltage        = 3900,   // the battery voltage at which the RPi will be started again
		.i2c_config_register_16bit.warn_voltage           = 3400,   // the battery voltage at which the RPi should should down
		.i2c_config_register_16bit.ups_shutdown_voltage   = 3200,   // the battery voltage at which a hard shutdown is executed
};

I2C_Status_Register_8Bit _i2c_status_register_8bit = {
	.charger_contact         =    0,   // if != 0 contact with the charger chip has been established
	.charger_status          =    0,   // contains the contents of the status register 0x0E
	.ups_state               =    ups_unclear_state,
};

I2C_Status_Register_16Bit _i2c_status_register_16bit = {
	.ups_bat_voltage         =    0,   // the battery voltage, 3.3 should be low and 3.7 high voltage
	.charge_current          =    0,   // the battery charge current
	.vbus_voltage            =    0,   // the primary power voltage
	.seconds                 =    0,   // seconds since last i2c access
	.temperature             =    0,   // the on-chip temperature
};
