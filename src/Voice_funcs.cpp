#include <Arduino.h>
#include "global_vars.h"
#include <pwm.h>
#include <Code_parse.h>

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
            play_voice(63);
            random_play_num=random(9,12);
        }else if(receive_voice_condition==4){//清除所有指令
            if(mode_switch_condition==1)face_condition=1;
            else if(mode_switch_condition==2)face_condition=6;
            symbol_counter=0;
            current_symbol=0;
            for(int i=0;i<20;i++)symbol_array[i]=0;
            code_str_raw="&";
            random_play_num=random(12,15);
        }else if(receive_voice_condition==5){//撤销指令
            // current_symbol=0;
            // symbol_counter--;这里去掉了，改为发送'F',0是删除
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
            //random_play_num=random(41,46);
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
            vTaskSuspend(TFT_TASK_Handle);
            TFT_instant_stop=true;
            vTaskResume(TFT_TASK_Handle);
            face_condition=0;
            remote_or_code_mode=0;
            random_play_num=random(86,88);
            for(int i=0;i<20;i++)symbol_array[i]=0;
            code_str_raw="&";
            code_str_raw_item_counter=0;
            code_str_clean = "";
            //把编程状态下的操作紧急停止且不发声音,不能像再ESPNOW_SLAVE.cpp中那样收到'R',O 把start_cypher=0;以及instant_stop=1;因为这样会让main程序里的部分发声。
            instant_stop=1;
            vTaskSuspend(Code_Process_Handle);//先把线程停下来
            code_str_clean="";//清空执行的程序
            vTaskResume(Code_Process_Handle);//由于code_str_clean清空以及instant_stop置为1，因此小车必然停下来，再开启线程，空跑完
            pwm_stop();//然后把车停下来
            vTaskDelay(100/portTICK_PERIOD_MS);
            instant_stop=0;

        }else if(receive_voice_condition==21){//切换为编程模式
            motor_speed=slow_speed;//速度并不是在发送W协议的时候设置的，而是在一开始切换模式的时候
            face_condition=1;
            remote_or_code_mode=1;
            random_play_num=random(88,90);
            for(int i=0;i<20;i++)symbol_array[i]=0;
            code_str_raw="&";
            code_str_raw_item_counter=0;
            code_str_clean = "";
        }else if(receive_voice_condition==22){//加速卡触发
            random_play_num=random(94,96);
        }else if(receive_voice_condition==23){//加速卡结束
            random_play_num=96;
        }else if(receive_voice_condition==24){//减速触发
            random_play_num=random(97,99);
        }else if(receive_voice_condition==25){//旋转卡触发
            random_play_num=101;
        }else if(receive_voice_condition==26){//混乱卡
            random_play_num=random(102,104);
        }else if(receive_voice_condition==27){//陷阱卡
            play_voice(104);
            random_play_num=105;
        }else if(receive_voice_condition==28){//状态恢复
            random_play_num=random(99,101);
        }else if(receive_voice_condition==29){//胜利！
            play_voice(random(64,67));
            random_play_num=random(106,109);
        }else if(receive_voice_condition==30){//切换为生存模式
            random_play_num=random(109,111);
            motor_speed=slow_speed;//速度并不是在发送W协议的时候设置的，而是在一开始切换模式的时候
            face_condition=6;
            remote_or_code_mode=1;
            random_play_num=random(109,111);
            for(int i=0;i<20;i++)symbol_array[i]=0;
            code_str_raw="&";
            code_str_raw_item_counter=0;
            code_str_clean = "";
        }else if(receive_voice_condition==31){//第1关开始
            random_play_num=200;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==32){//第1关结束第2关开始
            random_play_num=201;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==33){//第2关结束第3关开始
            random_play_num=202;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==34){//第3关结束第4关开始
            random_play_num=203;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==35){//第4关结束第5关开始
            random_play_num=204;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==36){//第5关结束第6关开始
            random_play_num=205;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==37){//第6关结束第7关开始
            random_play_num=206;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==38){//第7关结束第8关开始
            random_play_num=207;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==39){//第8关结束第9关开始
            random_play_num=208;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==40){//第9关结束
            random_play_num=209;
            code_parse_emergent_stop();
        }else if(receive_voice_condition==41){//陷阱type1
            random_play_num=211;//陷阱语音1
            code_parse_emergent_stop();
        }else if(receive_voice_condition==42){//陷阱type2
            random_play_num=212;//陷阱语音2
            code_parse_emergent_stop();
        }else if(receive_voice_condition==43){//陷阱type3
            random_play_num=213;//陷阱语音3
            code_parse_emergent_stop();
        }else if(receive_voice_condition==44){//陷阱type4
            random_play_num=214;//陷阱语音4
            code_parse_emergent_stop();
        }else if(receive_voice_condition==45){//陷阱type5
            random_play_num=215;//陷阱语音5
            code_parse_emergent_stop();
        }else if(receive_voice_condition==50){//从新开始本关
            random_play_num=random(146,150);
        }else if(receive_voice_condition==51){//这个任务已经完成了，但是用户再次扫了这个任务卡，对应3播放过故事，且任务完成
            random_play_num=random(144,146);
        }else if(receive_voice_condition==52){//上一个任务还没开始，就扫描了下面关卡的任务点，请先完成上一个任务
            random_play_num=random(142,144);
        }else if(receive_voice_condition==53){//已经完成了整个任务
            random_play_num=141;
        }else if(receive_voice_condition==54){//无法解析代码，机体瘫痪,cannot_start_cypher==1 闯关模式 在ESPNOW中 需要返回任务点维修才能重新开始任务
            random_play_num=140;
        }else if(receive_voice_condition==55){//无法解析代码，燃料耗尽,cannot_start_cypher==2 生存模式 在ESPNOW中 需要返回基地补充能量才能开始任务
            random_play_num=random(138,140);
        }else if(receive_voice_condition==56){//无法解析代码，燃料耗尽,cannot_start_cypher==3 生存模式 在ESPNOW中触发 时间耗尽
            random_play_num=random(136,138);
        }else if(receive_voice_condition==57){//无法解析代码，生存模式已完成 在ESPNOW中触发  时间耗尽
            random_play_num=random(134,136);
        }else if(receive_voice_condition==71){
            random_play_num=220;//生存挑战模式介绍
        }else if(receive_voice_condition==72){
            random_play_num=221;//倒计时结束
            code_parse_emergent_stop();
        }else if(receive_voice_condition==73){
            random_play_num=222;//提示用户倒计时开始
        }else if(receive_voice_condition==74){
            random_play_num=223;//生存挑战完成
            code_parse_emergent_stop();
        }else if(receive_voice_condition==75){//1级矿物
            random_play_num=224;
        }else if(receive_voice_condition==76){//2级矿物
            random_play_num=225;
        }else if(receive_voice_condition==77){//3级矿物
            random_play_num=226;
        }else if(receive_voice_condition==78){//4级矿物
            random_play_num=227;
        }else if(receive_voice_condition==79){//5级矿物
            random_play_num=228;
        }
        play_voice(random_play_num);
        receive_voice_flag=false;
    }
}