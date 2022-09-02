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
int rfid_scan_loop_counter=0;
int motor_speed=255;
int full_speed=255;
int slow_speed=125;

int face_condition=0;//0-默认脸，1-编程脸
int current_symbol=0;//0-初始状态，不显示是什么符号，1-前进 2-左转 3-后退 4-右转 5-左平移 6-右平移 7-循环2 8-循环3 9-循环结束 10-条件1开始 11-条件1结束 12-条件2开始 13-条件2结束 14-条件3开始 15-条件3结束
int symbol_counter=0;
int symbol_array[20]={0};//记录都有哪些指令应该被显示，上限暂时设置为20
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
    }
    
    //  vTaskDelete(NULL);
  }
}

void RFID_TASK(void *parameters)
{
  for (;;)
  {

    rfid_scan_card();
    rfid_scan_loop_counter++;
    if(rfid_scan_loop_counter==20){rfid_scan_loop_counter=0;send_data_now('0',0);}//每1秒钟检测一次是否连接控制器，更新connected_with_controller
    vTaskDelay(50 / portTICK_PERIOD_MS);
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
  }
}
void disconnect_funcs(){
  send_data_now('0',0);
  if(first_time_play_disconnected_voice==true){
    int random_play_num=random(90,92);
    play_voice(random_play_num);
    first_time_play_disconnected_voice=false;
    first_time_play_connected_voice=true;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(25);
  rfid_init();
  pwm_init(pwmA1, pwmA2, pwmB1, pwmB2, pwmC1, pwmC2, pwmD1, pwmD2);
  espnow_slave_init();
  TFT_func_init();
  xTaskCreate(TFT_TASK, "TFT_TASK", 5000, NULL, 1, NULL);
  xTaskCreate(RFID_TASK, "RFID_TASK", 5000, NULL, 1, NULL);
  volume_read_memory();
  //
  //robot_startUp();
}

void loop()
{
  // esp_now_send(0, (uint8_t *)&test, sizeof(test));//广播
  // esp_now_send(masteraddress, (uint8_t *)&send_data, sizeof(send_data));
 
  if (connected_with_controller == true)
  {
    connected_funcs();
    pwm_receive_esp_now_behaviors();
    voice_receive_esp_now_behaviors();
    card_process(rfid_block_buffer);
  }else if(connected_with_controller==false){
    disconnect_funcs();
  }

  Serial_commands();
}