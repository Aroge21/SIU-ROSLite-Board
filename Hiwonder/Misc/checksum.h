/**
 *@file checksum.h
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief Function declarations of several verification algorithms
 *@version 0.1
 *@date 2023-05-20
 *
 *@copyright Copyright (c) 2023
 *
 */
 
#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <stdio.h>
#include <stdint.h>

/**
*@defgroup Checksum Checksum
*@{
*/

/**
*@brief accumulation and verification
*@param buf Data to be calculated
*@param len Number of bytes of data to be calculated
*@return Accumulation and result, only the lower eight digits are valid
*/
uint16_t checksum_sum(const uint8_t *buf, uint16_t len);

/**
 *@brief Exclusive Or verification
 *@details Calculate the XOR result of all bytes
 *@param buf Data to be calculated
 *@param len Number of bytes of data to be calculated
 *@return XOR verification result, only the lower eight digits are valid
 */
uint16_t checksum_xor(const uint8_t *buf, uint16_t len);  

/**
 *@brief CRC8 verification
 *@details CRC8 verification of the input data and return the verification result
 *@param buf Data to be calculated
 *@param len Number of bytes of data to be calculated
 *@return CRC8 verification result, only the lower 8 bits are valid
 */
uint16_t checksum_crc8(const uint8_t *buf, uint16_t len);/*CRC8 Verification */


/**
 *@brief CRC16 verification
 *@details CRC16 verification of the input data and return the verification result
 *@param buf Data to be calculated
 *@param len Number of bytes of data to be calculated
 *@return CRC16 verification result
 */
uint16_t checksum_crc16(const uint8_t *buf, uint16_t len);/*CRC16 Verification */

/**
 *@} 
 */

#endif/*__CHECK_H__ */
