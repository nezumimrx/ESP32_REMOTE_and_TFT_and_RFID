#ifndef _prfid_funcs_H_
#define _rfid_funcs_H_
#include <Arduino.h>
void rfid_init();
void read_block_ntag213(uint8_t blockAddr, byte (&block_buffer)[18]);
void rfid_scan_card();
void Serial_commands();
#endif




