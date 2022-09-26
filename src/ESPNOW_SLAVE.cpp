#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <CheckCode.h>
#include "global_vars.h"
#include "Voice_funcs.h"

typedef struct data_to_send
{
  int x;
  int y;
} data_to_send;
data_to_send send_data;

typedef struct data_to_recv
{
    char x[2];
    int y;
} data_to_recv;
data_to_recv received_data;
// peer address and declaration of peer info
//测试的时候两块板子地址要对调一下，otherguy和myaddress，不然就重复了

esp_now_peer_info_t peerInfo;
uint8_t symbol_add_or_delete=0;//0-无操作 1-添加 2-删除
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus)
{
    //char macStr[18]; //显示为FF:FF:FF:FF:FF:FF
    //snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    //Serial.print(macStr);
    //Serial.print("send status: ");
    if (sendStatus == 0)
    {
        connected_with_controller=true;
        //Serial.println("send sucess!");
    }
    else
    {
        connected_with_controller=false;
        //Serial.println("fail");
    }
}
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    
    memcpy(&received_data, incomingData, sizeof(received_data));
    // Serial.print("Bytes received: ");
    // Serial.println(len);
    // Serial.print("x: ");
    // Serial.println(received_data.x);
    // Serial.print("y: ");
    // Serial.println(received_data.y);
    // Serial.println();
    if(robot_started==true){//初始化完成遥控器的按键才有效
        if(*received_data.x=='W'){
            if(mode_switch_condition==0){
                receive_data_flag=true;
                if(received_data.y==0){receive_wheel_condition=0;remote_running=false;}
                else if(received_data.y==1){receive_wheel_condition=1;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
                else if(received_data.y==2){receive_wheel_condition=2;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
                else if(received_data.y==3){receive_wheel_condition=3;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
                else if(received_data.y==4){receive_wheel_condition=4;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
                else if(received_data.y==5){receive_wheel_condition=5;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
                else if(received_data.y==6){receive_wheel_condition=6;remote_running=true;receive_voice_flag=true;receive_voice_condition=14;}
               
            }else if((mode_switch_condition==1||mode_switch_condition==2)&&received_data.y!=0){
                vTaskSuspend(TFT_TASK_Handle);
                TFT_instant_stop=true;
                vTaskResume(TFT_TASK_Handle);
                face_condition=2;//记录指令
                current_symbol=received_data.y;
                if(current_symbol!=0&&current_symbol!=19&&current_symbol!=20)symbol_add_or_delete=1;
                else if(current_symbol==19&&current_symbol!=20&&symbol_counter>=1) {symbol_add_or_delete=2;}//19-撤销，20-清除
                else if(current_symbol==20){receive_voice_flag=true;receive_voice_condition=4;}
            }

        }else if(*received_data.x=='v'){
            receive_voice_flag=true;
            receive_voice_condition=received_data.y;
        }else if(*received_data.x=='0'){
            if(received_data.y==0)connected_with_controller=true;
        }else if(*received_data.x=='s'){
            if(received_data.y==0)motor_speed=full_speed;
            else if(received_data.y==1)motor_speed=slow_speed;
        }else if(*received_data.x=='f'){//是哪种表情 //0-默认脸 //1-编程脸//2-显示箭头 //3-angry //4-happy //5-sad
            face_condition=received_data.y;
        }else if(*received_data.x=='F'&&(mode_switch_condition==1||mode_switch_condition==2)){//接收编程指令 current_symbol 0-要删除上一个指令 1-前进 2-左转 3-后退 4-右转 5-左平移 6-右平移 7-循环2 8-循环3 9-循环结束 10-条件1开始 11-条件1结束 12-条件2开始 13-条件2结束 14-条件3开始 15-条件3结束
            vTaskSuspend(TFT_TASK_Handle);
            TFT_instant_stop=true;
            vTaskResume(TFT_TASK_Handle);
            face_condition=2;//记录指令
            current_symbol=received_data.y;
            if(current_symbol!=0&&current_symbol!=19&&current_symbol!=20)symbol_add_or_delete=1;//!=0是因为默认会在W1后面发送W0，所以0不能用
            else if(current_symbol==19&&current_symbol!=20&&symbol_counter>=1) {symbol_add_or_delete=2;}//删除指令
            else if(current_symbol==20){receive_voice_flag=true;receive_voice_condition=4;}//清空指令

        }else if(*received_data.x=='R'&&(mode_switch_condition==1||mode_switch_condition==2)){
            if(received_data.y==1){//接收到运行代码指令
                if(cannot_start_cypher==0)check_code(code_str_raw);//如果检查没有问题，会将start_cypher变为1 cannot_start_cypher == 0 正常解析
                else if(cannot_start_cypher==1){receive_voice_flag=true;receive_voice_condition=54;}//机体完蛋
                else if(cannot_start_cypher==2){receive_voice_flag=true;receive_voice_condition=55;}//生存模式，燃料耗尽
                else if(cannot_start_cypher==3){receive_voice_flag=true;receive_voice_condition=56;}//生存模式，时间耗尽，由RFID_TASK里的tick触发
                else if(cannot_start_cypher==4){receive_voice_flag=true;receive_voice_condition=57;}//生存模式已通关

            }else if(received_data.y==0&&instant_stop==0&&start_cypher==1){//紧急停止
                instant_stop = 1;
                start_cypher = 0;
            }
        }else if(*received_data.x=='M'){
            if(received_data.y==1){//接收到切换模式的指令
                mode_switch_condition++;
                if(mode_switch_condition>=3)mode_switch_condition=0;
                if(mode_switch_condition==0){receive_voice_flag=true;receive_voice_condition=20;cannot_start_cypher=0;survive_time_counter_start=false;}//切换为遥控
                else if(mode_switch_condition==1){receive_voice_flag=true;receive_voice_condition=21;cannot_start_cypher=0;survive_time_counter_start=false;}//切换为编程
                else if(mode_switch_condition==2){receive_voice_flag=true;receive_voice_condition=30;survive_mode_intro=false;survive_time_counter=0;survive_fuel=1;survive_collected_points=0;cannot_start_cypher=0;survive_time_counter_start=false;}//切换为生存挑战
            }
        }else if(*received_data.x=='V'){
            if(received_data.y==1){//接收到调节音量的
                if(volume==15){volume=0;pref.begin("volume",false);pref.putInt("vol",volume);pref.end();change_volume(volume);}
                else if(volume==0){volume=27;pref.begin("volume",false);pref.putInt("vol",volume);pref.end();change_volume(volume);play_voice(63);}
                else if(volume==27){volume=20;pref.begin("volume",false);pref.putInt("vol",volume);pref.end();change_volume(volume);play_voice(63);}
                else if(volume==20){volume=15;pref.begin("volume",false);pref.putInt("vol",volume);pref.end();change_volume(volume);play_voice(63);}
            }
        }
    }
}

void espnow_slave_init()
{
    // change MAC address
    WiFi.mode(WIFI_STA);
    esp_wifi_set_mac(WIFI_IF_STA, &myaddress[0]);
    Serial.print("current my add: ");
    Serial.println(WiFi.macAddress());
    WiFi.disconnect();
    // initialize ESP-NOW
    if (esp_now_init() != 0)
    {
        Serial.println("ESP-NOW fail and restart device.");
        return;
    }
    esp_now_register_send_cb(onDataSent);
    memcpy(peerInfo.peer_addr, masteraddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("failed to add peer");
    }
    esp_now_register_recv_cb(OnDataRecv);
}

void send_data_now(char c,int num){
  send_data.x = c;
  send_data.y = num;
  esp_now_send(masteraddress, (uint8_t *)&send_data, sizeof(send_data));
}
