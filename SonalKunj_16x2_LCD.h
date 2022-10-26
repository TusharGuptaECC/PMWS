#ifndef _SonalKunj_16x2_LCD_H_
#define _SonalKunj_16x2_LCD_H_

// lcd ///////////////////
#define RS_pin 3
#define RW_pin 4
#define E_pin 5
#define LCD_MODE 8
#define NEXT_LINE 0xC0
#define RETURN_HOME 0x02
#define CLEAR_DISPLAY 0x01

#include "SonalKunj_16x2_LCD.h"

// lcd functions ////////////////////////////////////////////////////////////////////////////////////////////
void initLcd(unsigned char);
void sendCmd(unsigned char bit, unsigned char);
void sendData(unsigned char bit,unsigned char);
void displayMsg(unsigned char *);

#endif
