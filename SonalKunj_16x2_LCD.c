#include "SonalKunj_16x2_LCD.h"

// lcd functions ////////////////////////////////////////////////////////////////////////////////////////////
void initLcd(unsigned char bit)
{
  DDRD = 0xFF;
  PORTD = 0xFF;
  DDRC |= 0x38;
  PORTC |= 0x38;
  _delay_ms(20);  // lcd takes approx. 15 ms to startup
  if (bit == 8)
    sendCmd(bit, 0x38); // 8-bit bit mode
  else
    sendCmd(bit, 0x28); // 4-bit bit mode
  sendCmd(bit, 0x06); // entry mode
  sendCmd(bit, 0x01); // clear display
  sendCmd(bit, 0x02); // return home
  sendCmd(bit, 0x0C); // display ON cursor OFF
}

void sendCmd(unsigned char bit, unsigned char cmd)
{

  PORTC &= ~( 1 << RS_pin );
  PORTC &= ~( 1 << RW_pin );

  if(bit == 8)
  {

    PORTD = cmd;

    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );
  }
  else
  {
    unsigned char temp;
    // sending upper nibble
    temp = cmd & 0xF0;
    PORTD &= 0x0F;
    PORTD |= temp; 
    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );

    //sending lower nibble
    cmd = (cmd << 4) & 0xF0;
    PORTD &= 0x0F;
    PORTD |= cmd; 
    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );
  }

}

void sendData(unsigned char bit,unsigned char data)
{
  PORTC |= ( 1 << RS_pin );
  PORTC &= ~( 1 << RW_pin );

  if (bit == 8)
  {
    PORTD = data;
    
    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );
  }

  else
  {
    unsigned char temp;
    // sending upper nibble
    temp = data & 0xF0;
    PORTD &= 0x0F;
    PORTD |= temp; 
    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );

    //sending lower nibble
    data = (data << 4) & 0xF0;
    PORTD &= 0x0F;
    PORTD |= data; 
    // sending enable pulse
    PORTC |= ( 1 << E_pin );
    _delay_ms(100);
    PORTC &= ~( 1 << E_pin );
  }
}

void displayMsg(unsigned char *msg)
{
  unsigned char index = 0;
  for(index = 0; msg[index] != '\0'; index++)
  {
    if (index == 16)
    {
      sendCmd(LCD_MODE, NEXT_LINE);
    }
    sendData(LCD_MODE, msg[index]);
  }
}
