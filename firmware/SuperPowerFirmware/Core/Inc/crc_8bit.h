/*
 * crc_8bit.h
 *
 *  Created on: Dec 10, 2020
 *      Author: jbaumann
 */

#ifndef INC_CRC_8BIT_H_
#define INC_CRC_8BIT_H_

static const uint8_t CRC8INIT = 0x00;                  // The initalization value used for the CRC calculation
static const uint8_t CRC8POLY = 0x31;                  // The CRC8 polynome used: X^8+X^5+X^4+X^0

uint8_t addCRC(uint8_t crc, uint8_t n);
uint8_t calcCRC(uint8_t reg, uint8_t *msg, uint8_t len);
#endif /* INC_CRC_8BIT_H_ */
