#ifndef _SonalKunj_switches_H_
#define _SonalKunj_switches_H_

#define B 'B'
#define C 'C'
#define D 'D'
#define HOME_pin 4
#define UP_pin 3
#define DOWN_pin 5

// io interrupts ///////
#define I_bit 7
#define EXTERNAL_INTERRUPT_EN 1
#define EXTERNAL_INTERRUPT_DIS 0
#define PIN_CHANGE_INTERRUPT_EN 1
#define PIN_CHANGE_INTERRUPT_DIS 0

unsigned char makeSwitch(unsigned char, unsigned char);
unsigned char readSwitch(unsigned char, unsigned char);

// Interrupts //
void enableIOInterrupt(unsigned char, unsigned char);
#endif
