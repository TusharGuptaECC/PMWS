#include "SonalKunj_switches.h"

unsigned char makeSwitch(unsigned char pOrt, unsigned char pIn)
{
    if (pOrt == D)
    {
        DDRD &= ~(1 << pIn);
        PORTD |= (1 << pIn);
    }
    else if (pOrt == B)
    {
        DDRB &= ~(1 << pIn);
        PORTB |= (1 << pIn);
    }
    else if (pOrt == C)
    {
        DDRC &= ~(1 << pIn);
        PORTC |= (1 << pIn);
    }
    else
    {
        return -1;
    }
    return 0;
}

unsigned char readSwitch(unsigned char pOrt, unsigned char pIn)
{
    unsigned char reg_read;
    if (pOrt == D)
    {
        reg_read = PIND & (1 << pIn);   
    }
    else if (pOrt == B)
    {
        reg_read = PINB & (1 << pIn);
    }
    else if (pOrt == C)
    {
        reg_read = PINC & (1 << pIn);
    }
    else
    {
        return -1;
    }
    return reg_read;
}

// Interrupts ////////////////////////////////////////////////////////////////////////////////////////
void enableIOInterrupt(unsigned char external_interrupt, unsigned char pin_changed_interrupt)
{
  SREG &= ~(1 << I_bit);  // disable all interrupts do avoid accidents
  if (external_interrupt)
  {
    // no external interrupt
    EICRA = 0x00; // 0b0000,0000 
    EIMSK = 0x00; // 0b0000,0000 unmasking the INT0
    // EIFR flag register
  }
  if (pin_changed_interrupt)
  {
    // yes pin change interrupts
    PCICR = 0x01; // 0b0000,0001 configure pin change at 0th group
    //PCIFR flag register for polling
    PCMSK2 = 0x00; // all are masked
    PCMSK1 = 0x00;  // all are masked
    PCMSK0 = 0x38;  // unmasking of d13, d12, d11
  }
  SREG |= (1 << I_bit);
}

