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
};

#endif





















