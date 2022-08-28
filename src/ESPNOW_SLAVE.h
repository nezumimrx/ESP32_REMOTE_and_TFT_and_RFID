
#ifndef _ESPNOW_SLAVE_H_
#define _ESPNOW_SLAVE_H_
#include <Arduino.h>
const uint8_t masteraddress[]={0xA0, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
const uint8_t myaddress[]={0xA0, 0x01, 0xAA, 0xAA, 0xAA, 0xAA};
void espnow_slave_init();
void send_data_now(char c,int num);

#endif
