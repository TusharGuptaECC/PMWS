#ifndef _SonalKunj_sleep_H_
#define _SonalKunj_sleep_H_

#define idle_mode 0x00
#define adc_noise_mode 0x02
#define power_down_mode 0x04
#define power_save_mode 0x06
#define standby_mode 0x0C
#define ext_standby_mode 0x0E

void initSleep(unsigned char);
void goToSleep(void);
void afterWakeUp(void);
//  void prepareForSleep(void);

#endif
