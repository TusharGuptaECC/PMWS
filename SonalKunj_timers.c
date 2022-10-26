#include "SonalKunj_timers.h"

void enableIcInterrupt(void)
{
    SREG &= ~(1 << I_bit);  // disable all interrupts do avoid accidents
    TIMSK1 = 0x21;  // input capture and overflow interrupt enable
    SREG |= (1 << I_bit);
}

void enableTimer2Interrupt(void)
{
    SREG &= ~(1 << I_bit);  // disable all interrupts do avoid accidents
    TIMSK2 = 0x01;  // overflow interrupt enable
    SREG |= (1 << I_bit); 
}

void pwmInit(void)
{
  // inverting 10 bit fast pwm
  TCCR1A = 0x33;  
  TCCR1B = 0x09;  // last three bits are pre-scaler here it is 1.
  TCCR1C = 0x00;  // for non-pwm mode only
  
  // for TCNT1 top is set to 0x03FF in 10-bit fast pwm mode
  //TCNT1H = ; 
  //TCNT1L = ;

  // OCR1A disabled i.e. no pwm on OC1A pin
  //OCR1AH = ;
  //OCR1AL = ;

  // default pwm is 50 %
  OCR1BH = 0x01;  // 0b0000,0000
  OCR1BL = 0x00;  // 0b0000,0000
  
  // no input capture
  // ICR1H = ;
  // ICR1L = ;
  
  // no interrupts
  TIMSK1 = 0x00;
  // TIFR1 = ;
}

void resetTimer1(void)
{
  // reset
  TCCR1A = 0x00;  
  TCCR1B = 0x00;  // prescaler starts the timer and stops it
  TCCR1C = 0x00;  // for non-pwm mode only
  TCNT1 = 0;
  // resetting can be done using GTCCR register also
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DHT11 ///////////////////////////////////////////////////////////////////////////////////////////////////
void enInputCap(void)
{
    TCCR1A = 0x00;  // normal mode
    TCCR1B = 0x41;  // trigger at positive edge with prescaler = 1
    TCCR1C = 0x00;  // used in output compare mode
}

/*
*	It starts the timer2 and also enables the timer2 interrupts.
*/
void initTimer2(void)
{
  TCCR2A = 0x00;
  TCCR2B = 0x07;  // clock pre-scaler = 1024 i.e. 64 us for one step
  //TCCR2C = 0x00;	// this register dosen't exist for timer2
  //ASSR |= (1 << 5);	// AS2 pin for external oscillator at tosc1 pin
  enableTimer2Interrupt();
}

/*
*	It resets the timer2
*/
void resetTimer2(void)
{
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  //TCCR2C = 0x00;	// this register dosen't exist for timer2
  TCNT2 = 0;
}

void pwm(unsigned char pump_speed)
{
  // default pwm is 50 %
  OCR1BH = 0x01;  // 0b0000,0000
  OCR1BL = 0x00;  // 0b0000,0000
  if (pump_speed == 0)
  {
    OCR1BH = 0x00;  // 0b0000,0000
    OCR1BL = 0x00;  // 0b0000,0000
  }
  if (pump_speed == 20)
  {
    OCR1BH = 0x01;  // 0b0000,0000
    OCR1BL = 0x00;  // 0b1100,1101
  }
  if (pump_speed == 40)
  {
    OCR1BH = 0x01;  // 0b0000,0001
    OCR1BL = 0x00;  // 0b1001,1010
  }
  if (pump_speed == 60)
  {
    OCR1BH = 0x01;  // 0b0000,0010
    OCR1BL = 0x00;  // 0b0110,0111
  }
  if (pump_speed == 80)
  {
    OCR1BH = 0x01;  // 0b0000,0011
    OCR1BL = 0x00;  // 0b0011,0100
  }
  if (pump_speed == 100)
  {
    OCR1BH = 0x01;  // 0b0000,0011
    OCR1BL = 0x00;  // 0b1111,1111
  }
}
