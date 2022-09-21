#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include <esp_now.h>
#include "pwm.h"
#include "global_vars.h"
#include "TFT_functions.h"
#include "Voice_funcs.h"
#include "Card_process.h"
#include <MFRC522.h>
#include <SPI.h>
#include <rfid_funcs.h>
#include <Code_parse.h>

Preferences pref;
int volume=15;
const int pwmA1 = 26;
const int pwmA2 = 27;
const int pwmB1 = 32;
const int pwmB2 = 33;
const int pwmC1 = 22;
const int pwmC2 = 21;
const int pwmD1 = 16;
const int pwmD2 = 17;
boolean receive_data_flag = false;
boolean receive_voice_flag = false;
int receive_wheel_condition = 0;
int receive_voice_condition = 0;
int temp_counter = 0;
String bmp_filename = "";
byte rfid_block_buffer[18] = {0};
boolean read_new_card = 0;

boolean connected_with_controller = false; //连没连上控制器
boolean first_time_play_disconnected_voice=true;//首次运行若未连接控制器则播放未连接语音
boolean first_time_play_connected_voice=true;//首次运行若已连接控制器则播放连接语音
int connection_loop_counter=0;
int motor_speed=175;
int full_speed=175;
int slow_speed=100;
boolean remote_running=false;
int remote_mode_stepped_card_condition=0;//0-没有踩到卡，1-踩到加速卡，2踩到减速卡，3踩到掉头卡，4踩到混乱卡，5踩到停顿卡，6-直行，7-左转，8-右转，9-循环，10-条件，99-胜利卡
int remote_mode_stepped_card_counter=0;//踩到卡的计时器，如果踩到，debuff持续3秒，buff持续5秒

int face_condition=0;//0-默认脸，1-编程脸
int current_symbol=0;//0-初始状态，不显示是什么符号，1-前进 2-左转 3-后退 4-右转 5-左平移 6-右平移 7-循环2 8-循环3 9-循环结束 10-条件1开始 11-条件1结束 12-条件2开始 13-条件2结束 14-条件3开始 15-条件3结束 19-删除
int symbol_counter=0;
int symbol_array[20]={0};//记录都有哪些指令应该被显示，上限暂时设置为20
boolean remote_or_code_mode=0;//0-remote mode 1-code mode


String code_str_raw="&";//所有添加+=";W1"之类的功能都在TFT_functions.cpp 为什么？因为要和屏幕同步，不然不同步
String code_str_clean="";
int code_str_raw_item_counter=0;
int code_str_raw_item_max=18;
String code_str_condition_type1="";
boolean has_condition_type1=false;
boolean receive_condition_type1=false;
String code_str_condition_type2="";
boolean has_condition_type2=false;
boolean receive_condition_type2=false;
String code_str_condition_type3="";
boolean has_condition_type3=false;
boolean receive_condition_type3=false;
boolean instant_stop=0;
boolean start_cypher=0;//先在main里分析代码正确与否再在线程Code_Process_TASK中处理code_parse

TaskHandle_t Code_Process_Handle;
TaskHandle_t TFT_TASK_Handle;
boolean TFT_instant_stop=false;

boolean robot_started=false;//初始化完，遥控器的按键才有效
int flash_emo_counter=0;
boolean flash_emo_counter_handle=false;
int flash_emo_time_max=20;//因为再RFID_TASK中所以是每50ms +1，那60就是3秒钟，flash_emo_持续3秒
int flash_emo_previous_face_condition=99;////用来储存在切换为flash_emo之前是什么表情

int mode_switch_condition;////0-遥控模式, 1 -编程闯关模式，2-编程积分模式

// Decare TASK 1
void TFT_TASK(void *parameters)
{
  for (;;)
  {
    if(face_condition==0){
      TFT_usualExpression();
    }else if(face_condition==1){
      TFT_waitforcode();
    }else if(face_condition==2){
      TFT_drawArrow();
    }else if(face_condition==3){
      TFT_angry();
    }else if(face_condition==4){
      TFT_happy();
    }else if(face_condition==5){
      TFT_sad();
    }else if(face_condition==6){
      TFT_venture();
    }else if(face_condition==7){
      TFT_win();
    }else if(face_condition==8){
      TFT_points();
    }else if(face_condition==9){
      TFT_noFuel();
    }else if(face_condition==10){
      TFT_noTime();
    }else if(face_condition==11){
      TFT_warning();
    }else if(face_condition==12){
      TFT_stage_clear();
    }else if(face_condition==13){
      TFT_forward();
    }else if(face_condition==14){
      TFT_left();
    }else if(face_condition==15){
      TFT_right();
    }
    
    
    //  vTaskDelete(NULL);
  }
}
//这里之前使用线程处理rfid的，结果发现扫描不够快，反而是电机不需要那么快响应，所以交换了一下，把电机的处理和声音的处理放进来了
void RFID_TASK(void *parameters)
{
  for (;;)
  {
    pwm_receive_esp_now_behaviors();
    voice_receive_esp_now_behaviors();
    
    connection_loop_counter++;
    if(connection_loop_counter==30){connection_loop_counter=0;send_data_now('0',0);}//每1.5秒钟检测一次是否连接控制器，更新connected_with_controller
    if(remote_mode_stepped_card_condition!=0&&enviroment_counter_short_stop==false){remote_mode_stepped_card_counter++;}//遥控模式下的卡踩到就开始计时
    if(flash_emo_counter_handle==true){
      flash_emo_counter++;
      if(flash_emo_counter>=60){
        face_condition=flash_emo_previous_face_condition;
        flash_emo_counter=0;flash_emo_counter_handle=false;flash_emo_previous_face_condition=99;//变量复位，为了保证下一次flash_emo的运行
      }
    }
    if(survive_time_counter_start==true){
      survive_time_counter++;
      if(survive_time_counter>=survive_time_limit){
        cannot_start_cypher=3;//时间耗尽，再次收到'R'语音会提示当前时间已耗尽，请回到任务起始位置重新开始
        receive_voice_flag=true;
        receive_voice_condition=72;
        survive_time_counter_start=false;
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
//
void Code_Process_TASK(void *parameters){
  for(;;){
    if(start_cypher==true){
      code_parse(code_str_clean);
      start_cypher=0;
      //play voice mission complete! and emo_mission_complete
      if(instant_stop==0&&step_on_right_card_when_start_cypher==false){
        receive_voice_flag=true;
        receive_voice_condition=13;
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}


void robot_startUp()
{
  int random_startup_soundeffect = random(67, 70); //音效
  int random_startup_voice = random(81, 84);       //语音
  play_voice(random_startup_soundeffect);
  delay(4000);
  play_voice(random_startup_voice);
  delay(4500);
  send_data_now('0',0);//测试是否连接控制器 若未连接onDataSent会把connected_with_controller置为false 
}

void connected_funcs(){
  if(first_time_play_connected_voice==true){
    int random_play_num=random(92,94);
    play_voice(random_play_num);
    first_time_play_connected_voice=false;
    first_time_play_disconnected_voice=true;
    for(int i=0;i<20;i++)symbol_array[i]=0;
    code_str_raw="&";
    code_str_raw_item_counter=0;
    code_str_clean = "";
    robot_started=true;//到这里才算初始化完
  }
}
void disconnect_funcs(){
  send_data_now('0',0);
  if(first_time_play_disconnected_voice==true){
    int random_play_num=random(90,92);
    play_voice(random_play_num);
    first_time_play_disconnected_voice=false;
    first_time_play_connected_voice=true;
    for(int i=0;i<20;i++)symbol_array[i]=0;
    code_str_raw="&";
    code_str_raw_item_counter=0;
    code_str_clean = "";
    face_condition=0;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(25);
  pwm_init(pwmA1, pwmA2, pwmB1, pwmB2, pwmC1, pwmC2, pwmD1, pwmD2);
  TFT_func_init();
  rfid_init();
  volume_read_memory();//
  espnow_slave_init();
  xTaskCreate(RFID_TASK, "RFID_TASK", 10000, NULL, 1, NULL);
  xTaskCreate(Code_Process_TASK, "Code_Process_TASK", 7000, NULL, 2, &Code_Process_Handle);
  xTaskCreate(TFT_TASK, "TFT_TASK", 10000, NULL, 1, &TFT_TASK_Handle);
  //robot_startUp();

  
}

void loop()
{
  // esp_now_send(0, (uint8_t *)&test, sizeof(test));//广播
  // esp_now_send(masteraddress, (uint8_t *)&send_data, sizeof(send_data));
 
  if (connected_with_controller == true)
  {
    connected_funcs();
    rfid_scan_card();
    card_process(rfid_block_buffer);
    if(instant_stop==1){
      vTaskSuspend(Code_Process_Handle);//先把线程停下来
      code_str_clean="";//清空执行的程序
      vTaskResume(Code_Process_Handle);//由于code_str_clean清空以及instant_stop置为1，因此小车必然停下来，再开启线程，空跑完
      // play voice emergent stop and play emo_stop
      receive_voice_flag = true;
      receive_voice_condition = 3; //紧急停止
      //
      pwm_stop();//然后把车停下来
      delay(500);
      instant_stop=0;
    }
  }else if(connected_with_controller==false){
    disconnect_funcs();
  }

  Serial_commands();
}