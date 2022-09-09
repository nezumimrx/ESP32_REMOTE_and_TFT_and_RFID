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
    //场地卡
    byte trap_card_speedup[2] = {0xC2,0x11};//加速卡
    byte trap_card_slowdown[2]= {0xC2,0x12};//减速卡
    byte trap_card_turn[2]= {0xC2,0x13};//掉头
    byte trap_card_confuse[2]= {0xC2,0x14};//混乱卡
    byte trap_card_stop[2]={0xC2,0x15};//停顿卡
    byte trap_card_winning[2]= {0xC2,0x01};//胜利卡
    byte trap_card_forward[2]= {0xC2,0x16};//前进卡
    byte trap_card_left[2]= {0xC2,0x17};//左转卡
    byte trap_card_right[2]= {0xC2,0x18};//右转卡

};

#endif





















