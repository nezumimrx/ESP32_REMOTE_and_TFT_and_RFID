#ifndef DataBlocks_h
#define DataBlocks_h

#include "Arduino.h"
class DataBlocks
{
   public:
    DataBlocks( );
    //第一种条件卡片 对应{}
    byte condition_type1[2] = { 0xC1, 0x11};
    byte condition_type2[2] = { 0xC1, 0x12};
    byte condition_type3[2] = { 0xC1, 0x13};
    //遥控模式场地卡
    byte trap_card_speedup[2] = {0xC2,0x11};//加速卡
    byte trap_card_slowdown[2]= {0xC2,0x12};//减速卡
    byte trap_card_turn[2]= {0xC2,0x13};//掉头
    byte trap_card_confuse[2]= {0xC2,0x14};//混乱卡
    byte trap_card_stop[2]={0xC2,0x15};//停顿卡
    byte trap_card_winning[2]= {0xC2,0x01};//胜利卡
    //遥控模式指令卡
    byte trap_card_forward[2]= {0xC2,0x16};//前进卡
    byte trap_card_left[2]= {0xC2,0x17};//左转卡
    byte trap_card_right[2]= {0xC2,0x18};//右转卡
    byte trap_card_loop2[2]={0xC2,0x19};//循2次环卡
    byte trap_card_loop3[2]={0xC2,0x20};//循3次环卡

    byte trap_card_condition[2]={0xC2,0x31};//条件卡
    //闯关\积分模式场地卡
    byte code_1_card_1[2]={0xC3,0x11};//第1关开始
    byte code_1_card_2[2]={0xC3,0x12};//第1关结束，第2关开始
    byte code_1_card_3[2]={0xC3,0x13};//第2关结束，第3关开始
    byte code_1_card_4[2]={0xC3,0x14};//第3关结束，第4关开始
    byte code_1_card_5[2]={0xC3,0x15};//第4关结束，第5关开始
    byte code_1_card_6[2]={0xC3,0x16};//第5关结束，第6关开始
    byte code_1_card_7[2]={0xC3,0x17};//第6关结束，第7关开始
    byte code_1_card_8[2]={0xC3,0x18};//第7关结束，第8关开始
    byte code_1_card_9[2]={0xC3,0x19};//第8关结束，第9关开始
    byte code_1_card_10[2]={0xC3,0x20};//第9关结束，通关
    byte code_trap_card_type1[2]={0xC3,0x01};//编程、生存模式陷阱卡1，对应不同语音
    byte code_trap_card_type2[2]={0xC3,0x02};//编程、生存模式陷阱卡2，对应不同语音
    byte code_trap_card_type3[2]={0xC3,0x03};//编程、生存模式陷阱卡3，对应不同语音
    byte code_trap_card_type4[2]={0xC3,0x04};//编程、生存模式陷阱卡4，对应不同语音
    byte code_trap_card_type5[2]={0xC3,0x05};//编程、生存模式陷阱卡5，对应不同语音
};

#endif





















