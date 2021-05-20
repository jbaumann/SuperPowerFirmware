/*
 * crc_8bit.h
 *
 *  Created on: Dec 10, 2020
 *      Author: jbaumann
 * This file contains the function prototypes for the CRC
 * algorithm.
 */

#ifndef INC_CRC_8BIT_H_
#define INC_CRC_8BIT_H_

uint8_t addCRC(uint8_t crc, uint8_t n);
uint8_t calcCRC(uint8_t reg, uint8_t *msg, uint8_t len);

#endif /* INC_CRC_8BIT_H_ */
