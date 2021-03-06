/*
 * i2c_register.h
 *
 *  Created on: Dec 5, 2020
 *      Author: jbaumann
 */

#ifndef INC_I2C_REGISTER_H_
#define INC_I2C_REGISTER_H_

#include "ups_state.h"

/*
 * The ConfigRegister structure holds all registers that are used
 * to set values that configure the uC. The StatusRegister structure
 * holds all registers that contain status information read by the
 * Raspberry Pi.
 * We use different structures for the 8-bit and the 16-bit values,
 * an additional structure for special registers and a final range
 * for the direct communication with tasks.
 * We use one byte as the register. We place the registers in the
 * following address range:
 * -  8-bit config 0x00-0x3F
 * -  8-bit status 0x40-0x7F
 * - 16-bit config 0x80-0xBF
 * - 16-bit status 0xC0-0xDF
 * - Special Regs. 0xE0-0xEF
 * - Task Comm.    0xF0-0xFF
 */

/*
 * We use an external program to extract the Register numbers automatically.
 * To do this we use a special marker comment _EXTRACT_I2C_REGISTER_, that
 * must not be removed. The names of the offsets and the structs must not
 * be changed. The pattern matching is pretty simple and relies on the naming.
 * Otherwise the extraction no longer works.
 * The numerical order of the OFFSET enums is important, we use it in i2c.c
 * to identify the correct struct.
 * We use our own #define Boolean to denote values that are used as boolean,
 * since otherwise we do not know which size the compiler picks for a _Bool.
 */

enum i2c_consts {
	I2C_BUFFER_SIZE      =   34,                          // max data size including register and crc
	CONFIG_8BIT_OFFSET   = 0x00,                          // _EXTRACT_I2C_REGISTER_
	STATUS_8BIT_OFFSET   = 0x40,                          // _EXTRACT_I2C_REGISTER_
	CONFIG_16BIT_OFFSET  = 0x80,                          // _EXTRACT_I2C_REGISTER_
	STATUS_16BIT_OFFSET  = 0xC0,                          // _EXTRACT_I2C_REGISTER_
	SPECIAL_16BIT_OFFSET = 0xE0,                          // _EXTRACT_I2C_REGISTER_
	TASK_COMMUNICATION   = 0xF0,                          // _EXTRACT_I2C_REGISTER_
};

typedef struct I2C_Config_Register_8Bit {                 // _EXTRACT_I2C_REGISTER_
	__IO uint8_t primed;
	__IO uint8_t force_shutdown;
	__IO uint8_t enable_bootloader;
	__IO uint8_t rtc_async_prediv;
	__IO uint8_t display_type;
} __attribute__((__packed__)) I2C_Config_Register_8Bit;   // _EXTRACT_I2C_REGISTER_

typedef struct I2C_Status_Register_8Bit {                 // _EXTRACT_I2C_REGISTER_
	__IO uint8_t charger_status;
	__IO uint8_t charger_contact;
	__IO enum UPS_State ups_state;
} __attribute__((__packed__)) I2C_Status_Register_8Bit;   // _EXTRACT_I2C_REGISTER_

typedef struct I2C_Config_Register_16Bit {                // _EXTRACT_I2C_REGISTER_
	__IO uint16_t timeout;
	__IO uint16_t restart_voltage;
	__IO uint16_t warn_voltage;
	__IO uint16_t ups_shutdown_voltage;
	__IO uint16_t rtc_sync_prediv;
} I2C_Config_Register_16Bit;  // _EXTRACT_I2C_REGISTER_

typedef struct I2C_Status_Register_16Bit {                // _EXTRACT_I2C_REGISTER_
	__IO uint16_t ups_bat_voltage;
	__IO uint16_t charge_current;
	__IO uint16_t vbus_voltage;
	__IO uint16_t seconds;
	__IO uint16_t temperature;
} I2C_Status_Register_16Bit;  // _EXTRACT_I2C_REGISTER_

/*
 * The following struct describes special registers of _arbitrary_ size.
 * The name is due to the limitations of the parsing program
 * extract_interface.py
 * The struct will never be instantiated, the transmission size has
 * to be coded by hand. See function HAL_I2C_AddrCallback()
 */
typedef struct I2C_Special_Register_16Bit {               // _EXTRACT_I2C_REGISTER_
	__IO uint8_t version;
	__IO uint8_t should_shutdown;
	__IO uint8_t write_to_eeprom;
	__IO uint8_t jump_to_bootloader;
} __attribute__((__packed__)) I2C_Special_Register_16Bit; // _EXTRACT_I2C_REGISTER_

/*
 * The struct declaration for the config registers. These are put into a backup
 * register structure so that we can write them to the RTC backup registers or
 * the eeprom without problems.
 * Special registers are _not_ instantiated.
 */

typedef struct {
	uint8_t version;
	I2C_Config_Register_8Bit  i2c_config_register_8bit;
	I2C_Config_Register_16Bit i2c_config_register_16bit;
//} __attribute__((__packed__)) Config_Registers;
} Config_Registers;

extern Config_Registers config_registers;

/*
 * The following assert guarantees that we have enough memory in the RTC Backup registers to store the data.
 * The error thrown (size of array... is negative" is not that clear but at least it gives an error at compile
 * time. The maximum value according to the datasheet is 80.
 */
#define ASSERT_BKUP_REG_SIZE(test) typedef char assertion_on_mystruct[( !!(test) )*2-1 ]
ASSERT_BKUP_REG_SIZE( (sizeof(config_registers)) < 80);

/*
 * We use pointers for all structs so we won't have to differentiate
 */
extern I2C_Config_Register_8Bit  *i2c_config_register_8bit;
extern I2C_Config_Register_16Bit *i2c_config_register_16bit;
extern I2C_Status_Register_8Bit  *i2c_status_register_8bit;
extern I2C_Status_Register_16Bit *i2c_status_register_16bit;
extern uint8_t  *i2c_config_register_8bit_reg;
extern uint16_t *i2c_config_register_16bit_reg;
extern uint8_t  *i2c_status_register_8bit_reg;
extern uint16_t *i2c_status_register_16bit_reg;

/*
 * Helper values allowing to check whether a register value is in bounds
 */
static const uint8_t i2c_config_reg_8bit_size   = sizeof(I2C_Config_Register_8Bit) / sizeof(uint8_t);
static const uint8_t i2c_status_reg_8bit_size   = sizeof(I2C_Status_Register_8Bit) / sizeof(uint8_t);
static const uint8_t i2c_config_reg_16bit_size  = sizeof(I2C_Config_Register_16Bit) / sizeof(uint16_t);
static const uint8_t i2c_status_reg_16bit_size  = sizeof(I2C_Status_Register_16Bit) / sizeof(uint16_t);
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
	i2creg_primed                  = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, primed),
	i2creg_force_shutdown          = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, force_shutdown),
	i2creg_enable_bootloader       = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, enable_bootloader),
	i2creg_rtc_async_prediv        = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, rtc_async_prediv),
	i2creg_display_type            = CONFIG_8BIT_OFFSET + offsetof(I2C_Config_Register_8Bit, display_type),
	// I2C_Status_Register_8Bit
	i2creg_charger_status          = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, charger_status),
	i2creg_charger_contact         = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, charger_contact),
	i2creg_ups_state               = CONFIG_8BIT_OFFSET + offsetof(I2C_Status_Register_8Bit, ups_state),
	// I2C_Config_Register_16Bit
	i2creg_timeout                 = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, timeout)/2,
	i2creg_restart_voltage         = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, restart_voltage)/2,
	i2creg_warn_voltage            = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, warn_voltage)/2,
	i2creg_ups_shutdown_voltage    = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, ups_shutdown_voltage)/2,
	i2creg_rtc_sync_prediv         = CONFIG_16BIT_OFFSET + offsetof(I2C_Config_Register_16Bit, rtc_sync_prediv)/2,
	// I2C_Status_Register_16Bit
	i2creg_ups_bat_voltage         = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, ups_bat_voltage)/2,
	i2creg_charge_current          = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, charge_current)/2,
	i2creg_vbus_voltage            = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, vbus_voltage)/2,
	i2creg_seconds                 = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, seconds)/2,
	i2creg_temperature             = STATUS_16BIT_OFFSET + offsetof(I2C_Status_Register_16Bit, temperature)/2,
	// I2C Special Registers
	i2creg_version                 = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, version),
	i2creg_should_shutdown         = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, should_shutdown),
	i2creg_write_to_eeprom         = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, write_to_eeprom),
	i2creg_jump_to_bootloader      = SPECIAL_16BIT_OFFSET + offsetof(I2C_Special_Register_16Bit, jump_to_bootloader),

}__attribute__ ((__packed__));            // force smallest size i.e., uint_8t (GCC syntax)


#endif /* INC_I2C_REGISTER_H_ */
