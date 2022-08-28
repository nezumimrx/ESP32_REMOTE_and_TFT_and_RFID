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
int receive_voice_condition =0;
int temp_counter = 0;
String bmp_filename = "";
byte rfid_block_buffer[18]={0};
boolean read_new_card=0;
// Decare TASK 1
void TFT_TASK(void *parameters)
{
  for (;;)
  {
    TFT_func_draw(18, 19);
    int random_facial_type=0;
    random_facial_type=random(1,10);
    if(random_facial_type==1)TFT_func_draw(1,12);
    else if(random_facial_type==2)TFT_func_draw(13,19);
    //TFT_func_draw_blinkeye();
    //TFT_func_draw_lookleftandright();
    // vTaskDelete(NULL);
  }
}

void RFID_TASK(void *parameters)
{
  for (;;)
  {
    rfid_scan_card();
    vTaskDelay(50 / portTICK_PERIOD_MS);
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
}



void loop()
{
  // esp_now_send(0, (uint8_t *)&test, sizeof(test));//广播
  // esp_now_send(masteraddress, (uint8_t *)&send_data, sizeof(send_data));
  pwm_receive_esp_now_behaviors();
  voice_receive_esp_now_behaviors();
  card_process(rfid_block_buffer);
  Serial_commands();
  Serial.println("test");
}