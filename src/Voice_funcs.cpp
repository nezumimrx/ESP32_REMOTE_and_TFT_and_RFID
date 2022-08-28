#include <Arduino.h>
#include "global_vars.h"

void play_voice(int num)
{
  byte playmusic[] = {0x7E, 0x05, 0x41, 0x00, 0x00, 0x45, 0xEF};
  byte highbyte = highByte(num);
  byte lowbyte = lowByte(num);
  byte confirmbyte = 0x05 ^ 0x41 ^ highbyte ^ lowbyte;
  playmusic[3] = highbyte;
  playmusic[4] = lowbyte;
  playmusic[5] = confirmbyte;
  Serial.write(playmusic, sizeof(playmusic));
  Serial.write("\r\n");
}

void voice_receive_esp_now_behaviors(){
    /*voice type 定义
        voice_type=1;//录入动作模块
        voice_type=2;//扫描了新模块但程序数量超出code_str_raw_item_max
        voice_type=3;//紧急停止
        voice_type=4;//清除所有指令
        voice_type=5;//撤销指令
        voice_type=6;//运行、启动、开始执行编程指令
        voice_type = 7; //程序有错误
        voice_type = 8; //没有录入程序指令
        voice_type=9;//条件开始命令
        voice_type=10;//条件结束命令
        voice_type=11;//循环起始命令
        voice_type=12;//循环结束命令
        voice_type=13;//程序运行完毕
        voice_type=14;//yoho
        */
    if (receive_voice_flag == true)
    {
        int random_play_num=0;
        if (receive_voice_condition == 1){//录入动作模块
            random_play_num=random(1,5);
        }else if(receive_voice_condition==2){//扫描了新模块但程序数量超出code_str_raw_item_max
            random_play_num=random(5,8);
        }else if(receive_voice_condition==3){//紧急停止
            random_play_num=random(9,12);
        }else if(receive_voice_condition==4){//清除所有指令
            random_play_num=random(12,15);
        }else if(receive_voice_condition==5){//撤销指令
            random_play_num=random(15,18);
        }else if(receive_voice_condition==6){//运行指令
            random_play_num=random(18,23);
        }else if(receive_voice_condition==7){//程序有误
            random_play_num=random(23,26);
        }else if(receive_voice_condition==8){//空指令
            random_play_num=random(26,29);
        }else if(receive_voice_condition==9){//条件开始命令
            random_play_num=random(29,32);
        }else if(receive_voice_condition==10){//条件结束命令
            random_play_num=random(32,35);
        }else if(receive_voice_condition==11){//条件结束命令
            random_play_num=random(35,38);
        }else if(receive_voice_condition==12){//条件结束命令
            random_play_num=random(38,41);
        }else if(receive_voice_condition==13){//条件结束命令
            random_play_num=random(41,46);
        }else if(receive_voice_condition==14){//条件结束命令
            random_play_num=random(46,52);
        }
        play_voice(random_play_num);
        receive_voice_flag=false;
    }
}