/*
 * i2c_register.h
 *
 *  Created on: Dec 5, 2020
 *      Author: jbaumann
 */

#ifndef INC_I2C_REGISTER_H_
#define INC_I2C_REGISTER_H_

/*
 * The ConfigRegister structure holds all registers that are used
 * to set values that configure the uC. The StatusRegister structure
 * holds all registers that contain status information read by the
 * Raspberry Pi.
 * We use different structures for the 8-bit and the 16-bit values,
 * thus all in all we have 4 register structures.
 * We use one byte as the register. We place the registers in the
 * following address range:
 * -  8-bit config 0x00-0x3F
 * -  8-bit status 0x40-0x7F
 * - 16-bit config 0x80-0xBF
 * - 16-bit status 0xC0-0xFF
 */

#define CONFIG_8BIT_OFFSET  0x00
#define STATUS_8BIT_OFFSET  0x40
#define CONFIG_16BIT_OFFSET 0x80
#define STATUS_16BIT_OFFSET 0xC0

typedef struct {
	volatile uint8_t primed;
	volatile uint8_t force_shutdown;
} I2C_Config_Register_8Bit;

typedef struct {
	volatile uint8_t should_shutdown;
} I2C_Status_Register_8Bit;

typedef struct {
	volatile uint16_t timeout;
	volatile uint16_t bat_voltage_coefficient;
	volatile int16_t  bat_voltage_constant;
	volatile uint16_t ext_voltage_coefficient;
	volatile int16_t  ext_voltage_constant;
	volatile uint16_t restart_voltage;
	volatile uint16_t warn_voltage;
	volatile uint16_t ups_shutdown_voltage;
	volatile uint16_t temperature_coefficient;
	volatile int16_t  temperature_constant;
} I2C_Config_Register_16Bit;

typedef struct {
	volatile uint16_t bat_voltage;
	volatile uint16_t ext_voltage;
	volatile uint16_t seconds;
	volatile uint16_t temperature;
} I2C_Status_Register_16Bit;

I2C_Config_Register_8Bit i2c_config_register_8bit;
I2C_Status_Register_8Bit i2c_statusregister_8bit;
I2C_Config_Register_16Bit i2c_config_register_16bit;
I2C_Status_Register_16Bit i2c_statusregister_16bit;

/*
 * We calculate the I2C Register number from the offset of the struct in the register range, adding
 * to it the offset of the respective value in the struct (divided by 2 for the 16bit values).
 * By doing this at compile time we minimize the possibility for errors in calculating the register
 * numbers.
 */
enum I2C_Register {
	// I2C_Config_Register_8Bit
	i2creg_primed                  = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, primed),
	i2creg_force_shutdown          = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, force_shutdown),
	// I2C_Status_Register_8Bit
	i2creg_should_shutdown          = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, should_shutdown),
	// I2C_Config_Register_16Bit
	i2creg_timeout                 = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, timeout)/2,
	i2creg_bat_voltage_coefficient = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, bat_voltage_coefficient)/2,
	i2creg_bat_voltage_constant    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, bat_voltage_constant)/2,
	i2creg_ext_voltage_coefficient = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, ext_voltage_coefficient)/2,
	i2creg_ext_voltage_constant    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, ext_voltage_constant)/2,
	i2creg_restart_voltage         = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, restart_voltage)/2,
	i2creg_warn_voltage            = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, warn_voltage)/2,
	i2creg_ups_shutdown_voltage    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, ups_shutdown_voltage)/2,
	i2creg_temperature_coefficient = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, temperature_coefficient)/2,
	i2creg_temperature_constant    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, temperature_constant)/2,
	// I2C_Status_Register_16Bit
	i2creg_bat_voltage             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, bat_voltage)/2,
	i2creg_ext_voltage             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, ext_voltage)/2,
	i2creg_seconds                 = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, seconds)/2,
	i2creg_temperature             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, temperature)/2,
}__attribute__ ((__packed__));            // force smallest size i.e., uint_8t (GCC syntax)


#endif /* INC_I2C_REGISTER_H_ */
