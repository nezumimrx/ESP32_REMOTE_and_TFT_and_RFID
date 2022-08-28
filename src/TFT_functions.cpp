#include <Arduino.h>
#include "global_vars.h"
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "blackbg.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library
File myFile;
int pixels_length=1408;
unsigned char RowBmp[1408];//128/8 * 高度
/*
在ESP32使用屏幕的时候，要打开TFT_eSPI库里的User_Setup.h把下面几行默认的备注做更改
#define ILI9341_DRIVER     //这个关上
#define ST7735_DRIVER      //这个打开
#define TFT_WIDTH  128     //打开
#define TFT_HEIGHT 128     //打开
#define ST7735_GREENTAB3   //打开重要！！不然有黑边
Typical setup for ESP8266 NodeMCU ESP-12 is :下面的都注释掉
#define TFT_MISO 19   //打开
#define TFT_MOSI 23   //打开
#define TFT_SCLK 18   //打开
#define TFT_CS   15  //打开
#define TFT_DC    2  //打开
#define TFT_RST   4  //打开
*/
void IRAM_ATTR DrawBmp(String name)
{
  myFile = SPIFFS.open(name, "r");
  // Serial.println(name);
  if (myFile)
  {

    while (myFile.available())
    {
      for (int i = 0; i < 0x3e; i++)
      {
        myFile.read();
      }
      for (int i = 0; i < 1408; i++)
      {
        RowBmp[i] = myFile.read();
      }
      tft.drawBitmap(0, 20, RowBmp, 128, 88, TFT_WHITE, TFT_BLACK);
      myFile.close();
    }
  }
  else
  {
    // Serial.print(F("error opening "));
    Serial.println(name);
  }
}

void TFT_func_init()
{
  tft.begin(); // Initialise the display
  tft.fillScreen(TFT_BLACK);
  tft.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
}

void TFT_func_draw(int start_index, int end_index){
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
  for (int i = end_index; i >= start_index; i--)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
}
void TFT_func_draw_blinkeye()
{
  for (int i = 1; i <= 16; i++)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
  for (int i = 16; i >= 1; i--)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
}

void TFT_func_draw_lookleftandright()
{
  for (int i = 11; i <= 21; i++)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
  for (int i = 21; i >= 11; i--)
  {
    String filename = "";
    filename = "/img (" + String(i) + ").bmp";
    DrawBmp(filename);
  }
}

