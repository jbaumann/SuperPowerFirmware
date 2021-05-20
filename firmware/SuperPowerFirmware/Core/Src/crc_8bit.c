/*
 * crc_8bit.c
 *
 *  Created on: Dec 10, 2020
 *      Author: jbaumann
 *
 * This file contains a calculation of a CRC based on the polynome used
 * by Dallas/Maxim. This CRC is used in the communication between RPi
 * and STM32.
 * These methods calculate an 8-bit CRC based on the polynome
 * (X^8+X^5+X^4+X^0).
 * The variables here don't need to be volatile because they are only accessed
 * during the interrupt in the I2C callback routines.
 */

#include "main.h"
#include "crc_8bit.h"

/*
 * These values define the polynome to be used
 * and the seed value, both are taken from
 * the Dallas / Maxim algorithm for OneWire.
 */
const uint8_t CRC8INIT = 0x00;                  // The initalization value used for the CRC calculation
const uint8_t CRC8POLY = 0x31;                  // The CRC8 polynome used: X^8+X^5+X^4+X^0


/*
 * This method adds a single byte to the CRC
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

/*
 * This method iterates over the msg with len bytes and returns
 * the CRC according to the Dallas polynome.
 */
uint8_t calcCRC(uint8_t reg, uint8_t *msg, uint8_t len) {
    uint8_t crc = addCRC(0, reg);

	for (int elem = 0; elem < len; elem++) {
        crc = addCRC(crc, msg[elem]);
	}
    return crc;

}

