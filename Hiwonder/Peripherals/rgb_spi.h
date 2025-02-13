
#ifndef __RGB_SPI_H
#define __RGB_SPI_H

#include <stdio.h>
#include <stdint.h>

#define Pixel_S1_NUM 2//Light beads RGB quantity

#define CODE0 0xC0//0 codes, sending time 1100 0000 Adjust appropriately according to different SCKs
#define CODE1 0xFC//1 code, sending time 1111 1100

void WS2812b_Configuration(void);

void rgb_SendArray(void);

void set_id_rgb_color(uint8_t id , uint8_t* rgb);
void set_rgb_color(uint8_t* rgb);

#endif//__RGB_SPI_H
