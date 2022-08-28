#include <Arduino.h>
#include "global_vars.h"
#include "Voice_funcs.h"
#include "DataBlocks.h"
#include <MFRC522.h>
#include <SPI.h>

#define rfid_rst_pin 0
#define rfid_sda_pin 5
MFRC522 rfid(rfid_sda_pin, rfid_rst_pin);
boolean write_or_read = 0; // if 0 read, if 1 write
int what_rfid_block_to_write = 0;
DataBlocks data_blocks;

void rfid_init()
{
    SPI.begin();
    rfid.PCD_Init();
    bool rfid_initResult = rfid.PCD_PerformSelfTest(); //
    if (rfid_initResult)
        Serial.println("RFID READY!");
    else
        Serial.println("RFID NOT READY!");
}

void read_block_ntag213(uint8_t blockAddr, byte (&block_buffer)[18])
{
    byte block_size = sizeof(block_buffer);
    MFRC522::StatusCode read_status = (MFRC522::StatusCode)rfid.MIFARE_Read(0x06, block_buffer, &block_size);
    if (read_status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(rfid.GetStatusCodeName(read_status));
    }
    for (byte i = 0; i < 16; i++)
    {
        Serial.print(block_buffer[i], HEX);
    }
    Serial.println();
}
void write_block_ntag213(byte *dataBlock, uint8_t blockAddr)
{
    byte data_to_write[] = {dataBlock[0], dataBlock[1], 0x00, 0x00};
    int write_status = (MFRC522::StatusCode)rfid.MIFARE_Ultralight_Write(blockAddr, &data_to_write[0], 4);
    if (write_status != MFRC522::STATUS_OK)
    {
        Serial.print(F("NTAG213_Read() failed "));
        return;
    }
    else
        Serial.println(F("NTAG213_Write() OK "));
}

void rfid_scan_card()
{
    boolean is_new_card_present = rfid.PICC_IsNewCardPresent();
    boolean read_card_serial = rfid.PICC_ReadCardSerial();
    if (is_new_card_present && read_card_serial)
    {
        if (write_or_read == 0)
        {
            rfid_block_buffer[18] = {0};
            read_block_ntag213(0x06, rfid_block_buffer);
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
            read_new_card = 1;
        }
        else
        {
            Serial.println(what_rfid_block_to_write);
            if(what_rfid_block_to_write==1)write_block_ntag213(data_blocks.condition_type1, 0x06);//写入卡片改这里
            else if(what_rfid_block_to_write==2)write_block_ntag213(data_blocks.condition_type2, 0x06);//写入卡片改这里
            else if(what_rfid_block_to_write==3)write_block_ntag213(data_blocks.condition_type3, 0x06);//写入卡片改这里
        }
    }
}

void Serial_commands()
{
    while (Serial.available())
    {
        String c = Serial.readString();
        if (c.startsWith("w"))
        {

            Serial.print("即将录入的卡片为：");
            what_rfid_block_to_write = c.substring(1).toInt();
            Serial.print(what_rfid_block_to_write);
            Serial.println("....");
            write_or_read = 1;
        }
        else if (c.startsWith("r"))
        {

            Serial.println("放入读取卡片...");
            write_or_read = 0;
        }
        else if (c.startsWith("v"))
        {
            int num = c.substring(1).toInt();
            play_voice(num);
        }
    }
}
