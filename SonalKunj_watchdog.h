#ifndef _SonalKunj_watchdog_H_
#define _SonalKunj_watchdog_H_

#define _2k_cycles 0x00 // 16 ms
#define _4k_cycles 0x01 // 32 ms
#define _8k_cycles 0x02 // 64 ms
#define _16k_cycles 0x03    // 0.125 sec
#define _32k_cycles 0x04    // 0.250 sec
#define _64k_cycles 0x05    // 0.50 sec
#define _128k_cycles 0x06   // 1 sec
#define _256k_cycles 0x07   // 2 sec
#define _512k_cycles 0x20   // 4 sec
#define _1024k_cycles 0x21  // 8 sec

#define WDT_stopped 0x00
#define WDT_reset_mode 0x08
#define WDT_interrupt_mode 0x40 // 0100,0000 0010,0001
#define WDT_IandR_mode 0x48

void turnOffWdt(void);
void initWdt(unsigned char, unsigned char);

#endif
