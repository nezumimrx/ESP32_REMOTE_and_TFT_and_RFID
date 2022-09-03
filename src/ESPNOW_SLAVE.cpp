#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "global_vars.h"
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
    //Serial.print("x: ");
    //Serial.println(received_data.x);
    //Serial.print("y: ");
    //Serial.println(received_data.y);
    //Serial.println();
    if(*received_data.x=='W'){
        receive_data_flag=true;
        if(received_data.y==0){receive_wheel_condition=0;remote_running=false;}
        else if(received_data.y==1){receive_wheel_condition=1;remote_running=true;}
        else if(received_data.y==2){receive_wheel_condition=2;remote_running=true;}
        else if(received_data.y==3){receive_wheel_condition=3;remote_running=true;}
        else if(received_data.y==4){receive_wheel_condition=4;remote_running=true;}
        else if(received_data.y==5){receive_wheel_condition=5;remote_running=true;}
        else if(received_data.y==6){receive_wheel_condition=6;remote_running=true;}
    }else if(*received_data.x=='v'){
        receive_voice_flag=true;
        receive_voice_condition=received_data.y;
    }else if(*received_data.x=='0'){
        if(received_data.y==0)connected_with_controller=true;
    }else if(*received_data.x=='s'){
        if(received_data.y==0)motor_speed=full_speed;
        else if(received_data.y==1)motor_speed=slow_speed;
    }else if(*received_data.x=='f'){//是哪种表情 //0-默认脸 //1-编程脸
        face_condition=received_data.y;
    }else if(*received_data.x=='F'){
        face_condition=2;//记录指令
        current_symbol=received_data.y;
        if(current_symbol!=0)symbol_counter++;
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
