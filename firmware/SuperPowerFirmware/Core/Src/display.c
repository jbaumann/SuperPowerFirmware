/*
 * display.c
 *
 *  Created on: May 15, 2021
 *      Author: jbaumann
 */

#include "main.h"
#include "FreeRTOS.h"

#define U8G2_USE_DYNAMIC_ALLOC
#include "u8g2.h"
#include "u8g2_port.h"
#include "i2c_register.h"
#include "ups_state.h"

uint8_t *screen_buffer = NULL;
uint8_t *i2c_buffer = NULL;
uint8_t *bufferDMA;
u8g2_t u8g2;

//TODO Find another way to reference these registers without exposing them to the general code
extern I2C_Status_Register_16Bit _status_register_16_bit;
extern I2C_Status_Register_8Bit _status_register_8bit;

typedef struct Display_Line {
	uint8_t x, y;
	char *string;
	__IO void *value;
	uint8_t type;
} Display_Line;

typedef struct Display_Definition {
	uint8_t address;
	void (*init_function)(void);
	uint8_t num_lines;
	Display_Line lines[];
} Display_Definition;

char printbuffer[6];
char* print_seconds_since_last_contact() {
	uint16_t seconds = calc_seconds_since_last_contact();
	sprintf(printbuffer, "%05d", seconds);
	return printbuffer;
}

void ada_feather_r1_init() {
	u8g2_Setup_sh1107_i2c_seeed_128x128_f(&u8g2, U8G2_R1,
				u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
}
Display_Definition ada_feather_r1 = {
	.address = 0x3c,
	.init_function = &ada_feather_r1_init,
	.lines = {
		{ .x = 0, .y = 48, .string = "VB", .value = NULL, },
		{ .x = 64, .y = 48, .string = "VP", .value = NULL, },
		{ .x = 0, .y = 48 + 1*12, .string = "St", .value = NULL, },
		{ .x = 64, .y = 48 + 1*12, .string = "Sht", .value = NULL, },
		{ .x = 0, .y = 48 + 2*12, .string = "Seconds", .value = NULL, },
		{ .x = 24, .y = 48, .string = "%4d", .value = &(_status_register_16_bit.ups_bat_voltage), .type = 2 },
		{ .x = 64+32, .y = 48, .string = "%4d", .value = &(_status_register_16_bit.vbus_voltage), .type = 2 },
		{ .x = 24, .y = 48 + 1*12, .string = "0x%02x", .value = &(_status_register_8bit.ups_state), .type = 1 },
		{ .x = 64+32, .y = 48 + 1*12, .string = "0x%02x", .value = &(ups_state_should_shutdown), .type = 1 },
//		{ .x = 64, .y = 48 + 2*12, .string = "%05d", .value = &(_status_register_16_bit.seconds), .type = 2 },
		{ .x = 64, .y = 48 + 2*12, .value = &print_seconds_since_last_contact, .type = 0xFF },
	},
	.num_lines = 10,
};

void ada_feather_r2_init() {
	u8g2_Setup_sh1107_i2c_seeed_128x128_f(&u8g2, U8G2_R2,
				u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
}
Display_Definition ada_feather_r2 = {
	.address = 0x3c,
	.init_function = &ada_feather_r2_init,
	.lines = {
		{ .x = 32, .y = 10, .string = "V Bat", .value = NULL, },
		{ .x = 32, .y = 10 + 2*12, .string = "V USB", .value = NULL, },
		{ .x = 32, .y = 10 + 4*12, .string = "State", .value = NULL, },
		{ .x = 32, .y = 10 + 6*12, .string = "Shld Sht", .value = NULL, },
		{ .x = 32, .y = 10 + 8*12, .string = "Seconds", .value = NULL, },
		{ .x = 32, .y = 10 + 1*12, .string = "%4d", .value = &(_status_register_16_bit.ups_bat_voltage), .type = 2 },
		{ .x = 32, .y = 10 + 3*12, .string = "%4d", .value = &(_status_register_16_bit.vbus_voltage), .type = 2 },
		{ .x = 32, .y = 10 + 5*12, .string = "0x%02x", .value = &(_status_register_8bit.ups_state), .type = 1 },
		{ .x = 32, .y = 10 + 7*12, .string = "0x%02x", .value = &(ups_state_should_shutdown), .type = 1 },
//		{ .x = 32, .y = 10 + 9*12, .string = "%05d", .value = &(_status_register_16_bit.seconds), .type = 2 },
		{ .x = 32, .y = 10 + 9*12, .value = &print_seconds_since_last_contact, .type = 0xFF },
	},
	.num_lines = 10,
};

Display_Definition *display_definitions[] = {
		&ada_feather_r1,
		&ada_feather_r2,
};
uint8_t num_display_definitions = sizeof(display_definitions)/sizeof(Display_Definition *);


void init_display() {
	volatile uint8_t display_type = i2c_config_register_8bit->display_type;
	if(display_type != 0 && display_type <= num_display_definitions) {
		Display_Definition *display = display_definitions[display_type-1];
		if(screen_buffer == NULL) {
			screen_buffer = (uint8_t*) pvPortMalloc(512); //buffer used as a "matrix" to draw on the display
			i2c_buffer = (uint8_t*) pvPortMalloc(32); //buffer used for communication via i2c in this case
		}

		(*display->init_function)();

		u8g2_SetBufferPtr(&u8g2, screen_buffer);
		u8g2_SetI2CAddress(&u8g2, display->address);
		u8g2_InitDisplay(&u8g2);
		u8g2_SetPowerSave(&u8g2, 0);
		u8g2_ClearDisplay(&u8g2);
	}
}

void update_display() {
	volatile uint8_t display_type = i2c_config_register_8bit->display_type;
	if(display_type != 0 && display_type <= num_display_definitions) {
		Display_Definition *display = display_definitions[display_type-1];

		if (i2c_status_register_8bit->charger_contact) {

			do {
				u8g2_FirstPage(&u8g2);
				u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);

				for (uint8_t l = 0; l < display->num_lines; l++) {
					Display_Line line = display->lines[l];
					if (line.value == NULL) {
						u8g2_DrawStr(&u8g2, line.x, line.y, line.string);
					} else {
						if(line.type < 0x80) {
							// we have a value that can be converted to a string
							char buffer[6];
							if(line.type == 1) {
								sprintf(buffer, line.string, *((uint8_t *)line.value));
							} else if(line.type == 2) {
								sprintf(buffer, line.string, *((uint16_t *)line.value));
							}
							u8g2_DrawStr(&u8g2, line.x, line.y, buffer);
						} else {
							if(line.type == 0x80) {
								// Glyph
								u8g2_DrawGlyph(&u8g2, line.x, line.y, *((uint16_t *)line.value));
							} else if(line.type == 0xFF) {
								// function
								char * (*my_function)(void) = line.value;
								char *buffer = (*my_function)();
								u8g2_DrawStr(&u8g2, line.x, line.y, buffer);
							}
						}
					}
				}
			} while (u8g2_NextPage(&u8g2));
		}
	}
}
