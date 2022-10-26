#include "SonalKunj_watchdog.h"

void turnOffWdt(void)
{
    asm("CLI\n\t; disable global interrupts"
        "WDR\n\t; reset wdt timer"
        );
    MCUSR &= ~(1 << WDRF);  // clearing wdt reset flag
    WDTCSR &= ~(1 << WDE);  // clearing WDE bit
    WDTCSR |= ((1 << WDCE) | (1 << WDE));   // enabling the time sequence
    WDTCSR = 0x00;  // turnoff the WDT timer
    asm("SEI\n\t; enable the global interrupts");
}

// Configures and start the WDT timer
void initWdt(unsigned char mode, unsigned char cycles)
{
    asm("CLI\n\t; disable global interrupts"
        "WDR\n\t; reset wdt timer"
        );
    MCUSR &= ~(1 << WDRF);  // clearing wdt reset flag
    WDTCSR &= ~(1 << WDE);  // clearing WDE bit
    WDTCSR |= ((1 << WDCE) | (1 << WDE));   // enabling the time sequence
    WDTCSR = 0x80| mode | cycles;    //1110,0001;
    asm("SEI\n\t; enable the global interrupts");
}
