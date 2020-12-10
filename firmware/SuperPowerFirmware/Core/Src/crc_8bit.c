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


///*
//   This function adds the current byte of data to the existing CRC calculation in the
//   variable reg.
//*/
//uint8_t crc8_bytecalc(uint8_t data, uint8_t reg)
//{
//  uint8_t i;                                           // we assume we have less than 255 bytes data
//  uint8_t flag;                                        // flag for the MSB
//  uint8_t polynome = CRC8POLY;
//
//  // for each bit of the byte
//  for (i = 0; i < 8; i++) {
//    if (reg & 0x80) flag = 1;                          // Test MSB of the register
//    else flag = 0;
//    reg <<= 1;                                         // Move register 1 bit to the left and
//    if (data & 0x80) reg |= 1;                         // Fill the LSB with the next data bit
//    data <<= 1;                                        // next bit of data
//    if (flag) reg ^= polynome;                         // if flag == 1 (MSB set) then XOR with polynome
//  }
//  return reg;
//}
//
///*
//   This function calculates the CRC8 of a msg using the function crc8_bytecalc().
//*/
//uint8_t crc8_message_calc(uint8_t *msg, uint8_t len)
//{
//  uint8_t reg = CRC8INIT;
//  uint8_t i;
//  for (i = 0; i < len; i++) {
//    reg = crc8_bytecalc(msg[i], reg);      // calculate the CRC for the next byte of data and add it to reg
//  }
//  return crc8_bytecalc(0, reg);      // The calculation has to be continued for the bit length of the polynome with 0 values
//}


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

