/*
 * i2c_register.c
 *
 *  Created on: 3 Jan 2021
 *      Author: jbaumann
 */

#include"main.h"
#include "i2c_register.h"


I2C_Config_Register_8Bit  *i2c_config_register_8bit = &(config_registers.val.i2c_config_register_8bit);
I2C_Config_Register_16Bit *i2c_config_register_16bit = &(config_registers.val.i2c_config_register_16bit);
I2C_Status_Register_8Bit  _status_register_8bit, *i2c_status_register_8bit = &_status_register_8bit;
I2C_Status_Register_16Bit _status_register_16_bit, *i2c_status_register_16bit = & _status_register_16_bit;

/*
 * Initialization of the register structures
 * TODO find a more elegant way for the initialization
 */


Config_Registers config_registers = {
		// ID, derived from the version number
		.val.version = BACKUP_INIT_VALUE,
		// 8 Bit config registers
		.val.i2c_config_register_8bit.val.primed                  =    0,   // 1 if the uC should control the system
		.val.i2c_config_register_8bit.val.force_shutdown          =    0,   // 1 if the uC should shutdown the UPS if the voltage is too low (hard shutdown)

		// 16 Bit config registers
		.val.i2c_config_register_16bit.val.timeout                =  120,   // the timeout for the reset, should cover shutdown and reboot
		.val.i2c_config_register_16bit.val.restart_voltage        = 3900,   // the battery voltage at which the RPi will be started again
		.val.i2c_config_register_16bit.val.warn_voltage           = 3400,   // the battery voltage at which the RPi should should down
		.val.i2c_config_register_16bit.val.ups_shutdown_voltage   = 3200,   // the battery voltage at which a hard shutdown is executed
};

I2C_Status_Register_8Bit _i2c_status_register_8bit = {
	.val.should_shutdown         =    0,   // if != 0 contains the motivation for why the RPi should shutdown
	.val.charger_status          =    0,   // contains the contents of the status register 0x0E
};

I2C_Status_Register_16Bit _i2c_status_register_16bit = {
	.val.bat_voltage             =    0,   // the battery voltage, 3.3 should be low and 3.7 high voltage
	.val.charge_current          =    0,   // the battery charge current
	.val.vbus_voltage            =    0,   // the primary power voltage
	.val.ext_voltage             =    0,   // external voltage measured on PA0
	.val.seconds                 =    0,   // seconds since last i2c access
	.val.temperature             =    0,   // the on-chip temperature
};
