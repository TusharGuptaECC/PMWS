/*
	* Author: Tushar Gupta
	* Project: Plant Monitoring and Watering System (PMWS).
*/

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdint.h>
#include "SonalKunj_16x2_LCD.h"
#include "SonalKunj_sleep.h"
#include "SonalKunj_switches.h"
#include "SonalKunj_timers.h"
#include "SonalKunj_watchdog.h"

// PUMP ///////////////////////////
// pump status //
#define PUMP_ON 1
#define PUMP_OFF 0

// pump pins //
#define en_pump_pin 1
#define pump_pin 2

// pump speed //
#define PUMP_SPEED_PERCENT_0 0
#define PUMP_SPEED_PERCENT_20 20
#define PUMP_SPEED_PERCENT_40 40
#define PUMP_SPEED_PERCENT_60 60
#define PUMP_SPEED_PERCENT_80 80
#define PUMP_SPEED_PERCENT_100 100

// dht11 /////////////////////////////
#define dht_pin 0

// system delay/////////////////////
/*
 * This 46 ms is the minimum delay and is calculated by doing rigrous
   practical.
 * This delay is required otherwise system will not update the 
   is_pressed variable after the ISR i.e outside the ISR i.e 
   is_pressed not becomes 0 at once and get ignored so this delay
   is necessary.
  * But for ensuring the proper results wait = 200 ms.
*/
#define wait 300

// watchdog timer ////////////////////////////////////////////
#define ten_mins 75
#define five_mins 38
#define one_min 8

// global variables///////////////////////////////////////////

// display page messages ///////
unsigned char *welcome_msg1 = "     PMWSv1     ";
unsigned char *welcome_msg2 = "   powered by      SonalKunj";
unsigned char *home_page_opt1 = "   automatic";
unsigned char *home_page_opt2 = "   manual";
unsigned char *selection_pointer = ">>";
unsigned char *temp = "temp:";
unsigned char *humid = "humid:";
unsigned char *p_speed = "speed";
unsigned char *manual_msg1 = "U -> ON  :STATUS";
unsigned char *manual_msg2 = "D -> OFF : ";
unsigned char *manual_msg3 = "ON";
unsigned char *manual_msg4 = "OFF";
unsigned char digits[] = "0123456789";

// control and decision ////////
// is_pressed holds the status of home/enter button
volatile unsigned char is_pressed = 0;

// is_automatic decides whether the mode is automatic or manual
volatile unsigned char is_automatic = 1;

// ic_data holds the value from the input capture
volatile uint32_t ic_data = 0;

// index2 ensures proper storage of dht11 data
volatile int32_t index2 = 0;

// It helps in debugging. Its value is incremented when timer1 overflows
volatile uint32_t over_flow = 0;

// Its value is incremented when timer2 overflows.
volatile uint32_t over_flow_2 = 0;

// It holds the binary data from the DHT11.
volatile  unsigned char dht_data[42];

// It holds the current processed humidity data from the sensor.
uint32_t humid_data = 0;

// It holds the current processed temperature data from the sensor.
uint32_t temp_data = 0;

// It holds the previous processed humidity data from the sensor.
uint32_t prev_humid_data = 0;

// It holds the previous processed temperature data from the sensor.
uint32_t prev_temp_data = 0;

// This variable decides the time period for which pump supplies water to the plants.
int16_t motor_on_duration = 0;

// This variable holds the current speed of motor. 
char pump_speed = 0;

// This variable maintains the information about the ON or OFF status of the pump.
unsigned char pump_status = PUMP_OFF;

// This variable locks the system so it will not get triggered by the gust of winds.
unsigned char wind_lock = 1;

// This variable value is incremented when watchdog timer overflows.
// In case of power failure this variable set the system in automatic mode within 8 secs if there is no human intervention.
volatile unsigned char watch_dog_wakeup = 0;

// ISRs /////////////////////////////////////////////////////

/*
*	This ISR handles interrupts from the buttons.
*	It updates the value of is_pressed variable according to the button pressed.
*/
ISR(PCINT0_vect)
{
	if (readSwitch(B, HOME_pin) == 0)
	{
		is_pressed = 1;
	}
	if (readSwitch(B, UP_pin) == 0)
	{
		is_pressed = 2;
	}
	if (readSwitch(B, DOWN_pin) == 0)
	{
		is_pressed = 3;
	}
}

/*
*	This ISR handles interrupts from the input capture.
*/
ISR(TIMER1_CAPT_vect)
{
    //previous_data = ic_data;
    ic_data = ICR1;// + (over_flow * 65536);
	TCNT1 = 0;
    //index2++;
    // PORTC ^= (1 << 0);
	index2++;
}

/*
*	This ISR handles interrupts timer1 overflow.
*/
ISR(TIMER1_OVF_vect)
{
    over_flow++;
}

/*
*	This ISR handles interrupts timer2 overflow.
*/
ISR(TIMER2_OVF_vect)
{
	over_flow_2++;
	/*
	*	debugging code
	*/
	// PORTC ^= (1 << 0);
	// if(over_flow_2 > 100000)
	// {
	// 	PORTC |= (1 << 0);
	// }
}

/*
*	This ISR handles interrupts from the watchdog timer.
*	It also resets the timer.
*/
ISR(WDT_vect)
{
	/*
	*	If in 8 sec there is no human intervention then put PMWS in autmatic mode.
	*/
	if (!watch_dog_wakeup)
	{
		is_pressed = 1;
		is_automatic = 1;
	}
	// if (index2 == 42)
	// {
	// 	index2++;
	// }
	watch_dog_wakeup++;
    turnOffWdt();
}

// functions declarations
void welcomePage1(void);
void welcomePage2(void);
void homePage_1(void);
void homePage_2(void);
void manualPageOn(void);
void manualPageOff(void);
void automaticPage(unsigned char clear_scr);
void initPump(void);
void powerUpPump(unsigned char is_enable);
unsigned char getHumidityInt(void);
unsigned char getTemperatureInt(void);
void dht11(void);
void displayTempData(uint32_t t);
void displayHumidData(uint32_t h);
void theLogic(void);
void debug(unsigned char value);

/*
*	Main function is the entry point.
*/
int main()
{
	// enter variable prevent the triggering of the pump for the first time.
	// As for the first time previous temp and humid are set to zero so pump will always get triggered.
	unsigned char enter = 0;

	// debugging code
	/*
		DDRC |= (1 << 0);
		PORTC &= ~(1 << 0);
	*/

	// initialize buttons on given pins.
	makeSwitch(B, HOME_pin);
	makeSwitch(B, UP_pin);
	makeSwitch(B, DOWN_pin);
	
	// initialize the LCD in 8-bit mode.
	initLcd(LCD_MODE);
	
	// initialize the pump.
	initPump();
	
	// enables the interrupt functionality on the button pins.
	enableIOInterrupt(EXTERNAL_INTERRUPT_DIS,PIN_CHANGE_INTERRUPT_EN);
	
	// displays the first welcome message on the screen.
	welcomePage1();
	_delay_ms(1000);

	// displays the second welcome message on the screen.
	welcomePage2();
	_delay_ms(1000);

	// displays the home page on the screen.
	homePage_1();
	
	// If no human intervention in 8 sec then put system in automatic mode. Helpful in case of power failure.
	initWdt(WDT_interrupt_mode, _1024k_cycles);

	while (1)
	{
		if (is_pressed == 1)  // home is pressed
		{
			_delay_ms(wait);
			is_pressed = 0;
			if (is_automatic)	// user has selected automatic mode
			{
				while (1)
				{
					while (watch_dog_wakeup < ten_mins)	// sleep for given minutes
					{
						/*
							It clears the screen.
						*/
						sendCmd(8,CLEAR_DISPLAY);

						/*
							It brings the cursor to the first row and the first cloumn of the screen.
						*/
						sendCmd(8, RETURN_HOME);

						/*
							$ symbol implies system is in sleep mode.
							User has to press any button except the reset button for waking up the system.
						*/
						sendData(8, '$');

						/*
							Initializing the watchdog timer.
						*/
						initWdt(WDT_interrupt_mode, _1024k_cycles);

						/*
							It decides the sleep mode.
						*/
						initSleep(power_save_mode);

						/*
							system starts sleeping.
						*/
						goToSleep();

						/*
							It makes the required changes after the wakeup.
						*/
						afterWakeUp();

						// wind lock is open
						wind_lock = 1;
						if (is_pressed)	// any value but not 0 will break the loop as user can press any button to check humidity and temp.
						{
							_delay_ms(wait);
							// debug('g');
							is_pressed = 0;
							break;
						}
					} 
					// debug('h');
					
					// updating the previous values with the current values.
					prev_humid_data = humid_data;
					prev_temp_data = temp_data;

					//_delay_ms(2000);
					/*
					*	start input cap, input cap intrrupt en, reset timer1, end input cap
					*/
					dht11();  
					// debug('a');

					/*
						updating the values of temperature and humidity.
					*/
					humid_data = getHumidityInt();
					temp_data = getTemperatureInt();

					/*
						It displays the current temperature and humidity on the screen.
					*/
					automaticPage(0);
					// debug('b');

					// if there is any tempearture change trigger the pump.
					if ((temp_data - prev_temp_data != 0) && (enter) && (wind_lock))
					{
						wind_lock = 0;
						
						// pump_status has nothing to do with pump control it just prevent system from going into sleep mode. 
						pump_status = PUMP_ON;	// i.e. pump is on


						theLogic();
						// debug('c');
					}
					enter = 1;
					
					_delay_ms(wait);
					// original formula ((motor_on_duration * 1000) / 16))
					if (over_flow_2 > (motor_on_duration * 60))
					{
						_delay_ms(wait);
						powerUpPump(PUMP_OFF);
						pump_status = PUMP_OFF;	// i.e. pump is off
						automaticPage(1);
						resetTimer2();
						_delay_ms(wait);
						over_flow_2 = 0;
						//wind_detected = 0;
					}
					// debug('d');
					//sendCmd(8, 0x01); // clear screen
					sendCmd(8, 0x02); // return home
					if (is_pressed == 1)
					{
						_delay_ms(wait);
						is_pressed = 0;
						is_automatic = 1;
						homePage_1();
						powerUpPump(PUMP_OFF);
						break;
					}
					// debug('e');
					if (pump_status == PUMP_OFF)
					{
						watch_dog_wakeup = 1;
					}
					// debug('f');
				}
			}
			else
			{
				manualPageOff();
				while (1)
				{
					if (is_pressed == 1)
					{
						// // debugging code
						/*
							if (is_pressed == 1)
							{
							sendData(8, 0x41);
							}
							problem: some delay required
							solution: defined a wait
						*/
						_delay_ms(wait);
						is_pressed = 0;
						is_automatic = 1;
						homePage_1();
						break;
					}
					
					if (is_pressed == 2)
					{
						_delay_ms(wait);
						is_pressed = 0;
						powerUpPump(PUMP_ON);
						manualPageOn();
					}

					if (is_pressed == 3)
					{
						_delay_ms(wait);
						is_pressed = 0;
						powerUpPump(PUMP_OFF);
						manualPageOff();
					}
				}
			}
		}
		if (is_pressed == 2)  // up is pressed
		{
			_delay_ms(wait);
			is_pressed = 0;
			is_automatic = 1;
			homePage_1();
		}
		if (is_pressed == 3)  // down is pressed
		{
			_delay_ms(wait);
			is_pressed = 0;
			is_automatic = 0;
			homePage_2();
		}
	}
}

void welcomePage1(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(welcome_msg1);
}

void welcomePage2(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(welcome_msg2);
}

void homePage_1(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(home_page_opt1);
    sendCmd(LCD_MODE, NEXT_LINE);
    displayMsg(home_page_opt2);
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(selection_pointer);
}

void homePage_2(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(home_page_opt1);
    sendCmd(LCD_MODE, NEXT_LINE);
    displayMsg(home_page_opt2);
    sendCmd(LCD_MODE, NEXT_LINE);
    displayMsg(selection_pointer);
}

void manualPageOn(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(manual_msg1);
    sendCmd(LCD_MODE, NEXT_LINE);
    displayMsg(manual_msg2); 
    displayMsg(manual_msg3);
}

void manualPageOff(void)
{
    sendCmd(LCD_MODE, CLEAR_DISPLAY); // clear display
    sendCmd(LCD_MODE, RETURN_HOME);
    displayMsg(manual_msg1);
    sendCmd(LCD_MODE, NEXT_LINE);
    displayMsg(manual_msg2); 
    displayMsg(manual_msg4);
}

void automaticPage(unsigned char clear_scr)
{
	if (clear_scr)
	{
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendData(LCD_MODE, ' ');
		sendCmd(LCD_MODE, 0x02);	// return home
	}
	_delay_ms(100);
	
	displayMsg(temp);
	_delay_ms(100);
	displayTempData(temp_data);
	_delay_ms(100);
	// for(count = 0; count<= 39; count++)
	// {
	// 	dht_data[count] = 0;
	// }
	sendData(LCD_MODE, 0xDF);  // degree symbol
	sendData(LCD_MODE, 'C'); // Celcius symbol
	sendData(LCD_MODE, ' ');  // blank space
	_delay_ms(100);
	displayMsg(p_speed);
	
	sendData(LCD_MODE, ' ');  // blank space
	sendData(LCD_MODE, ' ');  // blank space
	_delay_ms(100);
	
	sendCmd(LCD_MODE, NEXT_LINE);
	_delay_ms(100);
	
	displayMsg(humid);
	_delay_ms(100);
	displayHumidData((humid_data));
	_delay_ms(100);
	
	sendData(LCD_MODE, 0x25); // % symbol
	_delay_ms(100);
}

// Pump Crtl ///////////////////////////////////////////////////////////////////////////////////////////////
void initPump(void)
{
	DDRB |= (1 << en_pump_pin);
	DDRB |= (1 << pump_pin); 
}

void powerUpPump(unsigned char is_enable)
{
	if (is_enable)
	{
		PORTB |= (1 << en_pump_pin);
		PORTB |= (1 << pump_pin);
	}
	else
	{
		PORTB &= ~(1 << en_pump_pin);
		PORTB &= ~(1 << pump_pin);
	}
}

unsigned char getHumidityInt(void)
{
    uint32_t humidityInt = 0, count;
    for(count=0;count<=7;count++)
    {
        humidityInt = humidityInt + dht_data[count]*pow(2,7-count);
    }
    return humidityInt;
}

unsigned char getTemperatureInt(void)
{
    uint32_t temperatureInt = 0, count;
    for(count=0;count<=7;count++)
    {
        temperatureInt = temperatureInt + dht_data[count+16]*pow(2,7-count);
    }
    return temperatureInt;
}

void dht11(void)
{
    DDRB |= (1 << dht_pin); // set dht_pin as output
    PORTB &= ~_BV(PORTB0);   // set PB0 as LOW
    _delay_ms(25);    // delay 25 miliseconds
    DDRB &= ~_BV(DDB0); // set PB0 as INPUT
    PORTB |= _BV(PORTB0); // set PB0 PULL-UP
    _delay_us(30);    // delay 30 microseconds
    PORTB &= ~_BV(PORTB0);  // remove PB0 PULL-UP
    enInputCap(); // initialize input capture
    enableIcInterrupt();// enable the interrupt
	//initWdt(WDT_interrupt_mode, _1024k_cycles);// saving from infinite loop if occured due to any reason
	while (index2 <= 42)
	{
		if (index2 >= 4)
		{
			if ((ic_data - 800) > 800)
			{
				dht_data[index2 - 4] = 1;
			}
			else
			{
				dht_data[index2 - 4] = 0;
			}
		}
	}
	// debug(digits[index2/10]);
	_delay_ms(1000);
	// debug(digits[index2%10]);
	_delay_ms(1000);
    resetTimer1();
    _delay_ms(500);
    over_flow = 0;
	index2 = -1;
}

void displayTempData(uint32_t t)
{
    if (t >= 100)
    {
        sendData(LCD_MODE, digits[1]);
        sendData(LCD_MODE, digits[0]);
        sendData(LCD_MODE, digits[0]); 
    }
    else if(t == 0)
    {
        sendData(LCD_MODE, digits[0]);
        sendData(LCD_MODE, digits[0]);
    }
    else
    {
        sendData(LCD_MODE, digits[t/10]);
        sendData(LCD_MODE, digits[t%10]);
    }
}

void displayHumidData(uint32_t h)
{
    if (h >= 100)
    {
        sendData(LCD_MODE, digits[1]);
        sendData(LCD_MODE, digits[0]);
        sendData(LCD_MODE, digits[0]); 
    }
    else if(h == 0)
    {
        sendData(LCD_MODE, digits[0]);
        sendData(LCD_MODE, digits[0]);
    }
    else
    {
        sendData(LCD_MODE, digits[h/10]);
        sendData(LCD_MODE, digits[h%10]);
    }
}

/*
*	This function decides the speed and on timings of pump.
*	It also displays the speed on the screen.
*/
void theLogic(void)
{
	int count = 0, count_inverse = 0;
	// only first temp change will trigger the reaction i.e windlock mechanism
	resetTimer2();
	_delay_ms(wait);
	over_flow_2 = 0;
	// actual formula (((temp_data - prev_temp_data) * 60) - ((humid_data / 10) * 10));
	motor_on_duration = (((temp_data - prev_temp_data) * 60) - (humid_data / 5)); 
	pump_speed = ((humid_data / 10) - (temp_data / 10));
	if(motor_on_duration < 0)
	{
		motor_on_duration *= -1;
	}
	if(pump_speed < 0)
	{
		pump_speed *= -1;
	}
	if (pump_speed <= 3)
	{
		pump_speed = 100; // 100%
		count = 4;
	}
	else if (pump_speed <= 5)
	{
		pump_speed = 80; // 80%
		count = 3;
	}
	else if (pump_speed <= 7)
	{
		pump_speed = 60; // 60%
		count = 2;
	}
	else if (pump_speed <= 10)
	{
		pump_speed = 40; // 40%
		count = 1;
	}
	else if (pump_speed > 10)
	{
		pump_speed = 20; // 20%
		count = 0;
	}
	else
	{
		pump_speed = 0; // 0%
		count = -1;
	}
	_delay_ms(100);
	sendData(LCD_MODE, ' ');  // blank space
	count_inverse = count;
	for(; count >= 0; count--)
	{
		sendData(LCD_MODE, 0xFF);
		_delay_ms(100);
	}
	for ( count = 0; count < (4 - count_inverse); count++)
	{
		sendData(LCD_MODE, ' ');	// bar
	}
	
	sendData(LCD_MODE, ' ');  // blank space
	_delay_ms(100);
	pwmInit();
	pwm(pump_speed); // 50 percent is default
	powerUpPump(PUMP_ON);
	initTimer2();
}

/*
*	This program helps in debugging
*/
void debug(unsigned char value)
{
	sendCmd(LCD_MODE, CLEAR_DISPLAY);
	sendCmd(LCD_MODE, RETURN_HOME);
	sendData(LCD_MODE, value);
}
