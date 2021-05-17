/*
 * display.c
 *
 *  Created on: May 15, 2021
 *      Author: jbaumann
 *
 *  This file contains the source code for the low-level handling of the display.
 *  Different display types can be defined programmatically and added to the
 *  array display_definitions, and the user can select from these by setting a
 *  configuration option "display type". If this type is set to 0, the display
 *  is not used. Otherwise, the value-1 is used as the index into the array to
 *  select the correct definition. It will be helpful to provide different
 *  orientations for displays (portrait and landscape) and optionally different
 *  font sizes.
 *  We restrict the font size change to display type definitions to balance the
 *  memory need and functionality.
 *
 * Every display definition consists of lines which are rendered onto the display.
 * Each line has an x and y position and contents (in string and value).
 * The content depends on the value of the type (which is an enum Display_Line_Types):
 * dlt_string   - a simple string (in string)
 * dlt_8bit     - an 8bit value (in v_ptr) that is rendered according to string
 * dlt_16bit    - a 16bit value (in v_ptr) that is rendered according to string
 * dlt_glyph    - a glyph (in value)
 * dlt_function - a string that is returned by the function referenced (in v_ptr)
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

/*
 * This is a kind of a hack to access variables that are private, because
 * C doesn't seem to allow accessing the address of the register variables
 * using a pointer
 */
extern I2C_Status_Register_16Bit _status_register_16bit;
extern I2C_Status_Register_8Bit _status_register_8bit;



/*
 * The Display_Line_Types define the allowed types of values for the line
 * definitions
 */
enum Display_Line_Types {
	dlt_string = 0,
	dlt_8bit = 1,
	dlt_16bit = 2,
	dlt_glyph = 0x80,
	dlt_function = 0xFF,
};

/*
 * The Display_line defines a single entry for rendering. If multiple entries
 * are in the same physical space later entries will be rendered on top of
 * the earlier ones.
 */
typedef struct Display_Line {
	uint8_t x, y;
	char *string;
	union {
		__IO void *v_ptr;
		__IO uint32_t value;
	};
	enum Display_Line_Types type;
} Display_Line;

/*
 * The Display_Definition holds the necessary information to use the display
 * and the list of Display_Line entries that will be rendered. It is important
 * to correctly set the value for num_lines because this will be used to
 * iterate through the lines.
 */
typedef struct Display_Definition {
	uint8_t address;
	void (*init_function)(void);
	const uint8_t *font;
	uint8_t num_lines;
	Display_Line lines[];
} Display_Definition;


/*
 * An example for a function that can be used in rendering information. In this
 * case we simply calculate the seconds since the last contact, render the
 * result into the static printbuffer and return a pointer to that.
 */
char printbuffer[6];
char* print_seconds_since_last_contact() {
	uint16_t seconds = calc_seconds_since_last_contact();
	sprintf(printbuffer, "%05d", seconds);
	return printbuffer;
}

/*
 * This is the definition for the Adafruit Featherwing OLED 64x128 in landscape mode.
 * Since currently there is no working definition for it we use the seeed128x128
 * with offsets to render in the visible area.
 * We start with the init function which sets up the landscape mode (orientation R1).
 * Then follows the display definition with address, reference to the init function,
 * used font and displayed lines. Last comes the number of lines. It is important to
 * set this to the correct value.
 * The order of lines is irrelevant, but since later lines will be rendered over earlier
 * ones if the use the same screen space rendering the changing values last makes
 * correcting errors easier.
 */
void ada_feather_landscape_init() {
	u8g2_Setup_sh1107_i2c_seeed_128x128_f(&u8g2, U8G2_R1,
				u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
}
Display_Definition ada_feather_landscape = {
	.address = 0x3c,
	.init_function = &ada_feather_landscape_init,
	.font = u8g2_font_unifont_t_symbols,
	.lines = {
		{ .x = 0, .y = 48, .string = "VB", .type = dlt_string, },
		{ .x = 64, .y = 48, .string = "VP", .type = dlt_string, },
		{ .x = 0, .y = 48 + 1*12, .string = "St", .type = dlt_string, },
		{ .x = 64, .y = 48 + 1*12, .string = "Sht", .type = dlt_string, },
		{ .x = 0, .y = 48 + 3*12, .value = 0x23F1, .type = dlt_glyph },
		{ .x = 24, .y = 48, .string = "%4d", .v_ptr = &(_status_register_16bit.ups_bat_voltage), .type = dlt_16bit },
		{ .x = 64+32, .y = 48, .string = "%4d", .v_ptr = &(_status_register_16bit.vbus_voltage), .type = dlt_16bit },
		{ .x = 24, .y = 48 + 1*12, .string = "0x%02x", .v_ptr = &(_status_register_8bit.ups_state), .type = dlt_8bit },
		{ .x = 64+32, .y = 48 + 1*12, .string = "0x%02x", .v_ptr = &(ups_state_should_shutdown), .type = dlt_8bit },
		{ .x = 24, .y = 48 + 3*12, .v_ptr = &print_seconds_since_last_contact, .type = dlt_function },
	},
	.num_lines = 10,
};

/*
 * This is the definition for the Adafruit Featherwing OLED 64x128 in portrait mode.
 * Since currently there is no working definition for it we use the seeed128x128
 * with offsets to render in the visible area.
 * We start with the init function which sets up the landscape mode (orientation R1).
 * Then follows the display definition with address, reference to the init function,
 * used font and displayed lines. Last comes the number of lines. It is important to
 * set this to the correct value.
 * The order of lines is irrelevant, but since later lines will be rendered over earlier
 * ones if the use the same screen space rendering the changing values last makes
 * correcting errors easier.
 */
void ada_feather_portrait_init() {
	u8g2_Setup_sh1107_i2c_seeed_128x128_f(&u8g2, U8G2_R2,
				u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
}
Display_Definition ada_feather_portrait = {
	.address = 0x3c,
	.init_function = &ada_feather_portrait_init,
	.font = u8g2_font_unifont_h_symbols,
	.lines = {
		{ .x = 32, .y = 10, .string = "V Bat", .type = dlt_string, },
		{ .x = 32, .y = 10 + 2*14, .string = "V USB", .type = dlt_string, },
		{ .x = 32, .y = 10 + 4*14, .string = "State", .type = dlt_string, },
		{ .x = 32, .y = 10 + 6*14, .string = "Shld Sht", .type = dlt_string, },
		{ .x = 32, .y = 10 + 8*14, .value = 0x23F1, .type = dlt_glyph },

		{ .x = 32, .y = 10 + 1*14, .string = "%4d", .v_ptr = &(_status_register_16bit.ups_bat_voltage), .type = dlt_16bit },
		{ .x = 32, .y = 10 + 3*14, .string = "%4d", .v_ptr = &(_status_register_16bit.vbus_voltage), .type = dlt_16bit },
		{ .x = 32, .y = 10 + 5*14, .string = "0x%02x", .v_ptr = &(_status_register_8bit.ups_state), .type = dlt_8bit },
		{ .x = 32, .y = 10 + 7*14, .string = "0x%02x", .v_ptr = &(ups_state_should_shutdown), .type = dlt_8bit },
		{ .x = 32 + 24, .y = 10 + 8*14, .v_ptr = &print_seconds_since_last_contact, .type = dlt_function },
	},
	.num_lines = 10,
};

/*
 * The array display_definitions contains available display definitions. The configuration
 * option "display type" is used to select the current display configuration.
 * If "display type" is 0 then no display configuration is used.
 * Else "disply type" - 1 is the index into the display_definitions array.
 */
Display_Definition *display_definitions[] = {
		&ada_feather_landscape,
		&ada_feather_portrait,
};
uint8_t num_display_definitions = sizeof(display_definitions)/sizeof(Display_Definition *);

////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Following are the functions that initialize the display and render the lines on it.
 *
 * Here be dragons...
 */


/*
 * Initialize the display according to the information provided in the currently selected
 * Display_definition.
 */
void init_display(uint8_t display_type) {
	//volatile uint8_t display_type = i2c_config_register_8bit->display_type;
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

/*
 * De-Init the display and release the allocated memory
 */
void deinit_display() {
	return;
	// TODO Verify that this works, check what else we have to release
	if(screen_buffer != NULL) {
		vPortFree(screen_buffer);
		screen_buffer = NULL;
		vPortFree(i2c_buffer);
		i2c_buffer = NULL;
	}
}

/*
 * Draw all lines defined in the currently active display_definition.
 * The actual drawing method depends on the type of the line.
 */
void update_display(uint8_t display_type) {
	//volatile uint8_t display_type = i2c_config_register_8bit->display_type;
	if(display_type != 0 && display_type <= num_display_definitions) {
		Display_Definition *display = display_definitions[display_type-1];

		if (i2c_status_register_8bit->charger_contact) {

			do {
				u8g2_FirstPage(&u8g2);
				u8g2_SetFont(&u8g2, display->font);

				for (uint8_t l = 0; l < display->num_lines; l++) {
					Display_Line line = display->lines[l];
					if (line.type == dlt_string) {
						u8g2_DrawStr(&u8g2, line.x, line.y, line.string);
					} else {
						if(line.type < dlt_glyph) {
							// we have a value that can be converted to a string
							char buffer[6];
							if(line.type == dlt_8bit) {
								sprintf(buffer, line.string, *((uint8_t *)line.v_ptr));
							} else if(line.type == dlt_16bit) {
								sprintf(buffer, line.string, *((uint16_t *)line.v_ptr));
							}
							u8g2_DrawStr(&u8g2, line.x, line.y, buffer);
						} else {
							if(line.type == dlt_glyph) {
								// Glyph
								u8g2_DrawGlyph(&u8g2, line.x, line.y, line.value);
							} else if(line.type == dlt_function) {
								// function
								char * (*my_function)(void) = line.v_ptr;
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
