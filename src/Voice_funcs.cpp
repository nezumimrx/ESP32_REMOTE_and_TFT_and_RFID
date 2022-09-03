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

void change_volume(int num){//0-30音量
    byte level = lowByte(num);
    byte changevolume[] = {0x7E, 0x04, 0x31, 0x19, 0x2C, 0xEF};
    byte confirmbyte= 0x04 ^ 0x31 ^ level;
    changevolume[3]=level;
    changevolume[4]=confirmbyte;
    Serial.write(changevolume,sizeof(changevolume));
    Serial.write("\r\n");
}

void volume_read_memory(){
  pref.begin("volume",false);//false-write/read true-read only
  int read_volume = pref.getInt("vol",0);//读取nvm存储的音量是多少
  if(read_volume==0)change_volume(volume);//如果是0，说明没改过设置，默认按0-30的第15档设置
  else change_volume(read_volume);//如果不是0，那么用户更改过音量设置，按上一次保存的音量运行
  pref.end();
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
            face_condition=1;
            symbol_counter=0;
            current_symbol=0;
            for(int i=0;i<20;i++)symbol_array[i]=0;
            random_play_num=random(12,15);
        }else if(receive_voice_condition==5){//撤销指令
            current_symbol=0;
            symbol_counter--;
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
        }else if(receive_voice_condition==13){//程序运行完毕
            int random_sound_effect=random(78,81);
            play_voice(random_sound_effect);
            random_play_num=random(41,46);
        }else if(receive_voice_condition==14){//小车遥控模式跑
            play_voice(63);
            random_play_num=random(46,52);
        }else if(receive_voice_condition==15){//触发第一种条件指令卡
            play_voice(63);//叮咚音效
            random_play_num=random(52,55);
        }else if(receive_voice_condition==16){//触发第二种条件指令卡
            play_voice(63);
            random_play_num=random(52,55);
        }else if(receive_voice_condition==17){//触发第三种条件指令卡
            play_voice(63);
            random_play_num=random(52,55);
        }else if(receive_voice_condition==18){//触发了条件但是没有录入任何条件指令
            play_voice(63);
            random_play_num=random(84,86);
        }else if(receive_voice_condition==19){//触发了条件但是没有录入对应的条件指令
            play_voice(63);
            random_play_num=random(55,57);
        }else if(receive_voice_condition==20){//切换到遥控模式
            motor_speed=full_speed;//速度并不是在发送W协议的时候设置的，而是在一开始切换模式的时候
            face_condition=0;
            random_play_num=random(86,88);
            for(int i=0;i<20;i++)symbol_array[i]=0;
        }else if(receive_voice_condition==21){//切换为编程模式
            motor_speed=slow_speed;//速度并不是在发送W协议的时候设置的，而是在一开始切换模式的时候
            face_condition=1;
            random_play_num=random(88,90);
            for(int i=0;i<20;i++)symbol_array[i]=0;
        }
        play_voice(random_play_num);
        receive_voice_flag=false;
    }
}