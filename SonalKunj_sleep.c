#include "SonalKunj_sleep.h"

void initSleep(unsigned char mode)
{
    SMCR = mode;
    MCUCR = 0x00;   // default settings
    PRR = 0xA7;
}

void goToSleep(void)
{
    SMCR |= (1 << SE);  // SE - sleep enable bit i.e. bit 0
    asm("SLEEP\n\t");
}

void afterWakeUp(void)
{
    SMCR &= ~(1 << SE);  // SE - sleep enable bit i.e. bit 0
    //asm("sleep");
}

// This function is device and requirement specific :)
// void prepareForSleep(void) 
// {
// 	// in this code we are using a deep sleep mode i.e. power save mode
// 	// so most of the input pins will be disabled but we have three pins in floating state
// 	// so we have to pull them up
// 	DDRC &= 0xF8;
// 	PORTC |= 0x07;
// 	// disabling the digital input buffer
// 	DIDR0 = 0x3F;
// 	DIDR1 = 0x03;
// }
