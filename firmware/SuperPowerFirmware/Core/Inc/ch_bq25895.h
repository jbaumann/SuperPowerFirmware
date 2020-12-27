/*
 * ch_bq25895.h
 * Here follow the definitions for the TI BQ25895
 *
 *  Created on: Dec 26, 2020
 *      Author: jbaumann
 */

#ifndef INC_CH_BQ25895_H_
#define INC_CH_BQ25895_H_

#include "main.h"

/*
 * Intervals for communication with the charger
 */
static const uint16_t ch_i2c_master_timeout =  1000;  // timeout in milliseconds
static const uint16_t ch_update_interval    = 10000;  // update interval for the charger in milliseconds
static const uint16_t ch_conv_delay         =  1000;  // time for conversion, see 8.2.8 Battery Monitor on p.24

/*
 * FunctionPrototypes
 */

HAL_StatusTypeDef ch_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef ch_transfer_byte_to_register(I2C_HandleTypeDef *hi2c, uint8_t i2c_register, uint8_t data);

uint16_t ch_convert_batv(uint8_t raw);
uint16_t ch_convert_vbus(uint8_t raw);
uint16_t ch_convert_charge_current(uint8_t raw);


/*
 * The charger's I2C address
 * Has to be shifted according to documentation
 */
static const uint8_t CHARGER_ADDRESS = 0x6A << 1;

/*
 * Register names and values which are not mapped in the following
 * register struct
 */
static const uint8_t CH_ILIM           = 0x00;
static const uint8_t CH_CONV_ADC       = 0x02;
static const uint8_t CH_CONFIG         = 0x03;
static const uint8_t CH_ICHG           = 0x04;
static const uint8_t CH_WATCHDOG       = 0x07;
static const uint8_t CH_BATFET         = 0x09;
static const uint8_t CH_STATUS         = 0x0B;

static const uint8_t CH_ILIM_MAX       = 0b00110000; // 3.25A input current limit, ILIM pin disabled
static const uint8_t CH_CONFIG_SYS_MIN = 0b00110000; // set SYS_MIN to 3.0V
static const uint8_t CH_WATCHDOG_STOP  = 0b10001101; // stop watchdog timer
static const uint8_t CH_ICHG_MAX       = 0b01001111; // 5.056A charging current limit
static const uint8_t CH_BATFET_DELAY   = 0b01001000; // delay before battery is disconnected

/*
 * Bit 7 - 0 stop conversion (d), 1 Start conversion
 * Bit 6 - 0 one shot (d), 1 continuous
 * Bit 5 - 0 1.5MHz Boost freq, 1 500KHz Boost freq (d)
 * Bit 4 - 0 Disable ICO Algorithm, 1 Enable ICO Algorithm (d)
 * Bit 3 - 0 Disable HVDCP handshake, 1 Enable HVDCP handshake (d)
 * Bit 2 - 0 Disable MaxCharge handshake, 1 Enable MaxCharge handshake (d)
 * Bit 1 - 0 Not in D+/D- or PSEL detection (d), 1 Force D+/D- detection
 * Bit 0 - 0 Disable D+/D- or PSEL detection when VBUS is plugged-in, 1 Enable (d)
 */
static const uint8_t CH_CONV_ADC_START = 0b10011101;
static const uint8_t CH_CONV_ADC_STOP  = 0b00011101;

/*
 * Register definitions for the BQ25895, starting with register
 * 0x0B and ending with 0x12, we don't read the others
 */

typedef union {
	struct _I2C_CH_BQ25895_Register {
		union {
			__IO uint8_t reg_0B;
			__IO uint8_t ch_status;
			struct {
				__IO uint8_t status_vsys : 1;
				__IO uint8_t status_stat : 1;
				__IO uint8_t status_sdp  : 1;
				__IO uint8_t status_chrg : 2;
				__IO uint8_t status_vbus : 3;
	        };
		};
		union {
			__IO uint8_t reg_0C;
			__IO uint8_t ch_faults;
			struct {
				__IO uint8_t fault_ntc   : 3;
				__IO uint8_t fault_bat   : 1;
				__IO uint8_t fault_chrg  : 2;
				__IO uint8_t fault_boost : 1;
				__IO uint8_t fault_wdog  : 1;
			};
		};
		union {
			__IO uint8_t reg_0D;
			__IO uint8_t ch_vindpm_threshold;
			struct {
				__IO uint8_t vindpm_threshold : 7;
				__IO uint8_t vindpm_force     : 1;
			};
		};
		union {
			__IO uint8_t reg_0E;
			__IO uint8_t ch_bat_voltage;
			struct {
				__IO uint8_t bat_voltage            : 7;
				__IO uint8_t bat_thermal_regulation : 1;
			};
		};
		union {
			__IO uint8_t reg_0F;
			__IO uint8_t ch_sys_voltage;
		};
		union {
			__IO uint8_t reg_10;
			__IO uint8_t ch_ts_voltage;
		};
		union {
			__IO uint8_t reg_11;
			__IO uint8_t ch_vbus_voltage;
			struct {
				__IO uint8_t vbus_voltage : 7;
				__IO uint8_t vbus_good    : 1;
			};
		};
		union {
			__IO uint8_t reg_12;
			__IO uint8_t ch_charge_current;
		};
	} __attribute__((__packed__)) val;
	uint8_t reg[sizeof(struct _I2C_CH_BQ25895_Register)];

} I2C_CH_BQ25895_Register;

/*
 * The actual struct declaration
 */
I2C_CH_BQ25895_Register i2c_ch_BQ25895_register;

#endif /* INC_CH_BQ25895_H_ */
