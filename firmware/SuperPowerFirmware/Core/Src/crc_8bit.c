/*
 * crc_8bit.c
 *
 *  Created on: Dec 10, 2020
 *      Author: jbaumann
 */

#include "main.h"
#include "crc_8bit.h"


/*
   These method calculate an 8-bit CRC based on the polynome used for Dallas / Maxim
   sensors (X^8+X^5+X^4+X^0).
   The variables here don't need to be volatile because they are only accessed
   during the interrupt in the I2C callback routines.
*/

uint8_t addCRC(uint8_t crc, uint8_t n) {
	for (int bit = 0; bit < 8; bit++) {
		if ((n ^ crc) & 0x80) {
			crc = (crc << 1) ^ CRC8POLY;
		} else {
			crc = (crc << 1);
		}
		n = n << 1;
	}
	return (crc & 0xFF);
}

uint8_t calcCRC(uint8_t reg, uint8_t *msg, uint8_t len) {
    uint8_t crc = addCRC(0, reg);

	for (int elem = 0; elem < len; elem++) {
        crc = addCRC(crc, msg[elem]);
	}
    return crc;

}

