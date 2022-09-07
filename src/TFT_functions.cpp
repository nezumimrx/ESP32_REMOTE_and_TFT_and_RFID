#include <Arduino.h>
#include "global_vars.h"
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "blackbg.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library
TFT_eSprite sprite = TFT_eSprite(&tft);

File myFile;
int pixels_length = 1408;
unsigned char RowBmp[1408];   // 128/8 * 高度
unsigned char SymbolBmp[124]; // 128/8 * 高度
int previous_face_condition=0;//
int previous_symbol_counter=0;//
int random_facial_type = 1;
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
      sprite.drawBitmap(0, 10, RowBmp, 128, 88, TFT_WHITE, TFT_BLACK);
      sprite.pushSprite(0, 0);
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
  tft.setRotation(6);
  tft.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
  sprite.createSprite(128, 128);

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
}

void TFT_func_draw(int start_index, int end_index)
{
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

void IRAM_ATTR DrawSymbol(int x, int y, int symbol)
{
  String filename="";
  filename="/symbol"+String(symbol)+".bmp";
  myFile = SPIFFS.open(filename, "r");
  // Serial.println(name);
  if (myFile)
  {

    while (myFile.available())
    {
      for (int i = 0; i < 0x3e; i++)
      {
        myFile.read();
      }
      for (int i = 0; i < 124; i++) // 32 * 31
      {
        SymbolBmp[i] = myFile.read();
      }
      sprite.drawBitmap(x, y, SymbolBmp, 30, 30, TFT_WHITE, TFT_BLACK);
      sprite.pushSprite(0, 0);
      myFile.close();
    }
  }
  else
  {
    // Serial.print(F("error opening "));
    Serial.println(filename);
  }
}

void IRAM_ATTR DrawCodingMode(int x, int y, String name)
{
  myFile = SPIFFS.open(name, "r");
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
      sprite.drawBitmap(x, y, RowBmp, 128, 88, TFT_BLACK, TFT_WHITE);
      sprite.pushSprite(0, 0);
      myFile.close();
    }
  }
  else
  {
    Serial.println(name);
  }
}



void TFT_drawArrow()
{
  if(previous_face_condition!=2){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=2;
  }
  if(current_symbol!=0&&previous_symbol_counter!=symbol_counter){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);//数据该更新了，要刷新黑屏
    sprite.pushSprite(0,0);
    
    for(int i=19;i>=1;i--){//因为要添加数据进来，所以除了第0位，从第1位开始所有数据后移一位
      symbol_array[i]=symbol_array[i-1];
    }
    symbol_array[0]=current_symbol;//每次新添加的都放到第0位
    for(int i=0;i<18;i++){
      int temp_symbol=symbol_array[i];
      if(temp_symbol!=0)DrawSymbol(30*i,50,temp_symbol);//显示symbolX.bmp
    }
    previous_symbol_counter=symbol_counter;
  }else if(current_symbol==0&&previous_symbol_counter!=symbol_counter){
    //由于current_symbol只在3个地方赋值，一个是clear，一个是收到'F'，一个是delete；收到F时不可能为0，clear时counter是0，因此只有delete会触发这个情况
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);//数据该更新了，要刷新黑屏
    sprite.pushSprite(0,0);
    if(symbol_counter>=1){
      for(int i=0;i<19;i++){//数组有20个，但是我们最多只寸18个symbol，因此18，19位都是0，再怎么循环到i+1也不会超过数组上限
        symbol_array[i]=symbol_array[i+1];
      }
    }
    if(symbol_counter<=0){
      face_condition=1;
      symbol_counter=0;
      for(int i=0;i<20;i++)symbol_array[i]=0;
    }
    for(int i=0;i<18;i++){
      int temp_symbol=symbol_array[i];
      if(temp_symbol!=0)DrawSymbol(30*i,50,temp_symbol);
    }
    previous_symbol_counter=symbol_counter;
  }
}

void TFT_waitforcode()
{
  if(previous_face_condition!=1){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=1;
  }
  DrawCodingMode(0, 20, "/symbolCode1.bmp");
  vTaskDelay(500 / portTICK_PERIOD_MS);
  DrawCodingMode(0, 20, "/symbolCode2.bmp");
  vTaskDelay(500 / portTICK_PERIOD_MS);
}

void TFT_eyeblink(int start_index, int end_index)
{
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/2blinkeye" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}
void TFT_wonder(int start_index, int end_index)
{
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/5Wonder" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}
void TFT_usual(String switchbehavior,int start_index, int end_index)
{
  //example：2usual10 (1).bmp
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/2usual" + switchbehavior +" ("+ String(i) + ").bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}

void TFT_usualExpression()
{
  if(previous_face_condition!=0){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=0;
  }
  int temp_facial_switch_to = int(random(1,200));
  if(random_facial_type==1){
    DrawBmp("/2usual01 (7).bmp");
    if(temp_facial_switch_to==1){
      TFT_usual("10",1,7);
      TFT_usual("01",1,7);
      random_facial_type=1;
    }else if(temp_facial_switch_to==2){
      TFT_usual("10",1,7);
      TFT_usual("02",1,6);
      random_facial_type=2;
    }else if(temp_facial_switch_to==3){
      TFT_usual("10",1,7);
      TFT_usual("03",1,6);
      random_facial_type=3;
    }
  }else if(random_facial_type==2){
    DrawBmp("/2usual02 (6).bmp");
    if(temp_facial_switch_to==1){
      TFT_usual("20",1,6);
      TFT_usual("01",1,7);
      random_facial_type=1;
    }else if(temp_facial_switch_to==2){
      TFT_usual("20",1,6);
      TFT_usual("02",1,6);
      random_facial_type=2;
    }else if(temp_facial_switch_to==3){
      TFT_usual("20",1,6);
      TFT_usual("03",1,6);
      random_facial_type=3;
    }
  }else if(random_facial_type==3){
    DrawBmp("/2usual03 (6).bmp");
    if(temp_facial_switch_to==1){
      TFT_usual("30",1,6);
      TFT_usual("01",1,7);
      random_facial_type=1;
    }else if(temp_facial_switch_to==2){
      TFT_usual("30",1,6);
      TFT_usual("02",1,6);
      random_facial_type=2;
    }else if(temp_facial_switch_to==3){
      TFT_usual("30",1,6);
      TFT_usual("03",1,6);
      random_facial_type=3;
    }
  }

}
