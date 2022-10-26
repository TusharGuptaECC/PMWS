#ifndef _SonalKunj_timers_H_
#define _SonalKunj_timers_H_

#include "SonalKunj_timers.h"

void enableIcInterrupt(void);
void enableTimer2Interrupt(void);
void pwmInit(void);
void resetTimer1(void);
void enInputCap(void);

/*
*	It starts the timer2 and also enables the timer2 interrupts.
*/
void initTimer2(void);

/*
*	It resets the timer2
*/
void resetTimer2(void);
void pwm(unsigned char);

#endif
