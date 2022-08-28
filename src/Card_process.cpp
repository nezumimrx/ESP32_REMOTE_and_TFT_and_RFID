#include "Card_process.h"
#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include "global_vars.h"

void enviroment_condition_cards(byte block_buffer[18]){
    if(block_buffer[1]==0x11){
        Serial.println("step on condition card type 1");
        send_data_now('C',1);
    }else if(block_buffer[1]==0x12){
        Serial.println("step on condition card type 2");
        send_data_now('C',2);
    }else if(block_buffer[1]==0x13){
        Serial.println("step on condition card type 3");
        send_data_now('C',3);
    }
}

void card_process(byte block_buffer[18])
{
    if (read_new_card)
    {
        if (block_buffer[0] == 0xC1)
        { // Enviroment CONDITION CARDS TYPE1 第一类条件卡片 对应{}中的指令
            enviroment_condition_cards(block_buffer);
        }
        // Serial.println(code_str_raw);
        //发送指令已添加声音
        read_new_card = false;
        for(int i=0;i<18;i++){rfid_block_buffer[i]=0x00;}
    }
}



