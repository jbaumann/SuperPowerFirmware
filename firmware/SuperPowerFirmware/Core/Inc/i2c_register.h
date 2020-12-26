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

/*
 * We use an external program to extract the Register numbers automatically.
 * To do this we use a special marker comment _EXTRACT_I2C_REGISTER_, that
 * must not be removed. The names of the offsets and the structs must not
 * be changed. The pattern matching is pretty simple and relies on the naming.
 * Otherwise the extraction no longer works.
 * The numerical order of the OFFSET enums is important, we use it in i2c.c
 * to identify the correct struct.
 */

enum i2c_consts {
	I2C_BUFFER_SIZE = 32,
	CONFIG_8BIT_OFFSET = 0x00,                         // _EXTRACT_I2C_REGISTER_
	STATUS_8BIT_OFFSET = 0x40,                         // _EXTRACT_I2C_REGISTER_
	CONFIG_16BIT_OFFSET = 0x80,                        // _EXTRACT_I2C_REGISTER_
	STATUS_16BIT_OFFSET = 0xC0,                        // _EXTRACT_I2C_REGISTER_
	SPECIAL_16BIT_OFFSET = 0xF0,                       // _EXTRACT_I2C_REGISTER_
};

typedef union {
	struct _I2C_Config_Register_8Bit {                 // _EXTRACT_I2C_REGISTER_
		__IO uint8_t primed;
		__IO uint8_t force_shutdown;
		__IO uint8_t enable_bootloader;
	} __attribute__((__packed__)) val;                 // _EXTRACT_I2C_REGISTER_
	uint8_t reg[sizeof(struct _I2C_Config_Register_8Bit)];

} I2C_Config_Register_8Bit;

typedef union {
	struct _I2C_Status_Register_8Bit {                 // _EXTRACT_I2C_REGISTER_
		__IO uint8_t should_shutdown;
		__IO uint8_t charger_status;
	} __attribute__((__packed__)) val;                 // _EXTRACT_I2C_REGISTER_
	uint8_t reg[sizeof(struct _I2C_Status_Register_8Bit)];
} I2C_Status_Register_8Bit;


typedef union {
	struct _I2C_Config_Register_16Bit {                // _EXTRACT_I2C_REGISTER_
		__IO uint16_t timeout;
		__IO uint16_t restart_voltage;
		__IO uint16_t warn_voltage;
		__IO uint16_t ups_shutdown_voltage;
	} __attribute__((__packed__)) val;                 // _EXTRACT_I2C_REGISTER_
	uint16_t reg[sizeof(struct _I2C_Config_Register_16Bit) / 2]; // adjust for 16 bit
} I2C_Config_Register_16Bit;

typedef union {
	struct _I2C_Status_Register_16Bit {                // _EXTRACT_I2C_REGISTER_
		__IO uint16_t bat_voltage;
		__IO uint16_t charge_current;
		__IO uint16_t vbus_voltage;
		__IO uint16_t ext_voltage;
		__IO uint16_t seconds;
		__IO uint16_t temperature;
	} __attribute__((__packed__)) val;                 // _EXTRACT_I2C_REGISTER_
	uint16_t reg[sizeof(struct _I2C_Status_Register_16Bit) / 2]; // adjust for 16 bit
} I2C_Status_Register_16Bit;

/*
 * The following struct describes special registers of _arbitrary_ size.
 * The name is due to the limitations of the parsing program
 * extract_interface.py
 * The struct will never be instantiated, the transmission size has
 * to be coded by hand. See function HAL_I2C_AddrCallback()
 */
typedef union {
	struct _I2C_Special_Register_16Bit {               // _EXTRACT_I2C_REGISTER_
		__IO uint8_t version;
		__IO uint8_t init_eeprom;
	} __attribute__((__packed__)) val;                 // _EXTRACT_I2C_REGISTER_
	uint16_t reg[sizeof(struct _I2C_Special_Register_16Bit)];
} I2C_Special_Register_16Bit;

/*
 * The actual struct declarations. Special registers are _not_ instantiated
 */
I2C_Config_Register_8Bit i2c_config_register_8bit;
I2C_Status_Register_8Bit i2c_status_register_8bit;
I2C_Config_Register_16Bit i2c_config_register_16bit;
I2C_Status_Register_16Bit i2c_status_register_16bit;

/*
 * Helper values allowing to check whether a register value is in bounds
 */
static const uint8_t i2c_config_reg_8bit_size = sizeof(i2c_config_register_8bit.reg) / sizeof(i2c_config_register_8bit.reg[0]);
static const uint8_t i2c_status_reg_8bit_size = sizeof(i2c_status_register_8bit.reg) / sizeof(i2c_status_register_8bit.reg[0]);
static const uint8_t i2c_config_reg_16bit_size = sizeof(i2c_config_register_16bit.reg) / sizeof(i2c_config_register_16bit.reg[0]);
static const uint8_t i2c_status_reg_16bit_size = sizeof(i2c_status_register_16bit.reg) / sizeof(i2c_status_register_16bit.reg[0]);
static const uint8_t i2c_special_reg_16bit_size = sizeof(I2C_Special_Register_16Bit);


/*
 * We calculate the I2C Register number from the offset of the struct in the register range, adding
 * to it the offset of the respective value in the struct (divided by 2 for the 16bit values).
 * By doing this at compile time we minimize the possibility for errors in calculating the register
 * numbers.
 * Important: Before using the values to access the members of the structures
 * the offset has to be subtracted.
 */
enum I2C_Register {
	// I2C_Config_Register_8Bit
	i2creg_primed                  = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, val.primed),
	i2creg_force_shutdown          = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, val.force_shutdown),
	// I2C_Status_Register_8Bit
	i2creg_should_shutdown         = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, val.should_shutdown),
	i2creg_charger_status          = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, val.charger_status),
	// I2C_Config_Register_16Bit
	i2creg_timeout                 = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, val.timeout)/2,
	i2creg_restart_voltage         = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, val.restart_voltage)/2,
	i2creg_warn_voltage            = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, val.warn_voltage)/2,
	i2creg_ups_shutdown_voltage    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, val.ups_shutdown_voltage)/2,
	// I2C_Status_Register_16Bit
	i2creg_bat_voltage             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.bat_voltage)/2,
	i2creg_charge_current          = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.charge_current)/2,
	i2creg_vbus_voltage            = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.vbus_voltage)/2,
	i2creg_ext_voltage             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.ext_voltage)/2,
	i2creg_seconds                 = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.seconds)/2,
	i2creg_temperature             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, val.temperature)/2,
	// I2C Special Registers
	i2creg_version                 = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, val.version),
	i2creg_init_eeprom             = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, val.init_eeprom),

}__attribute__ ((__packed__));            // force smallest size i.e., uint_8t (GCC syntax)


#endif /* INC_I2C_REGISTER_H_ */
