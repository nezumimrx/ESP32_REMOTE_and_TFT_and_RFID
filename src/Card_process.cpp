#include "Card_process.h"
#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include "global_vars.h"

int previous_receive_wheel_condition=0;//用来储存突发事件前小车的运行状态

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

void enviroment_remotebuff_cards(byte block_buffer[18]){//0-没有踩到卡，1-踩到加速卡，2踩到减速卡，3踩到掉头卡，4踩到混乱卡，5踩到停顿卡，99-胜利卡
    if(block_buffer[1]==0x11){//加速卡
        remote_mode_stepped_card_condition=1;
        remote_mode_stepped_card_counter=0;
    }else if(block_buffer[1]==0x12){//减速卡
        remote_mode_stepped_card_condition=2;
        remote_mode_stepped_card_counter=0;
    }else if(block_buffer[1]==0x13){//掉头卡
        remote_mode_stepped_card_condition=3;
        remote_mode_stepped_card_counter=0;
    }else if(block_buffer[1]==0x14){//混乱卡
        remote_mode_stepped_card_condition=4;
        remote_mode_stepped_card_counter=0;
    }else if(block_buffer[1]==0x15){//停顿卡
        remote_mode_stepped_card_condition=5;
        remote_mode_stepped_card_counter=0;
    }else if(block_buffer[1]==0x01){//胜利卡
        remote_mode_stepped_card_condition=99;
        remote_mode_stepped_card_counter=0;
    }
}

void card_process(byte block_buffer[18])
{
    if (read_new_card)
    {
        if (block_buffer[0] == 0xC1)
        { // Enviroment CONDITION CARDS TYPE1 第一类条件卡片 对应{}中的指令
            enviroment_condition_cards(block_buffer);
        }else if(block_buffer[0] == 0xC2){
            enviroment_remotebuff_cards(block_buffer);
        }
        // Serial.println(code_str_raw);
        //发送指令已添加声音
        read_new_card = false;
        for(int i=0;i<18;i++){rfid_block_buffer[i]=0x00;}
    }
}



