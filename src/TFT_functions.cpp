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
      if(symbol==1)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, TFT_GREEN, TFT_BLACK);//gbr的顺序，而不是RGB -绿
      else if(symbol==2)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, 0xfd7e ,TFT_BLACK);//红
      else if(symbol==3)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, TFT_YELLOW, TFT_BLACK);//黄
      else if(symbol==4)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, TFT_CYAN, TFT_BLACK);//蓝
      else if(symbol==5)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, 0xad7f, TFT_BLACK);
      else if(symbol==6)sprite.drawBitmap(x, y, SymbolBmp, 30, 30, 0xf707, TFT_BLACK);
      else sprite.drawBitmap(x, y, SymbolBmp, 30, 30, TFT_WHITE, TFT_BLACK);
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


String delete_last_item()
{
    String deleted_str = "";
    int last_delimiter_index = code_str_raw.lastIndexOf(";");
    deleted_str = code_str_raw.substring(0, last_delimiter_index);
    Serial.println(deleted_str);
    return deleted_str;
}

void TFT_drawArrow()
{
  if(previous_face_condition!=2){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=2;
  }
  if(current_symbol!=0&&previous_symbol_counter!=symbol_counter){//如果接到了有效的编程指令 current_symbol 0-要删除上一个指令 1-前进 2-左转 3-后退 4-右转 5-左平移 6-右平移 7-循环2 8-循环3 9-循环结束 10-条件1开始 11-条件1结束 12-条件2开始 13-条件2结束 14-条件3开始 15-条件3结束
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);//数据该更新了，要刷新黑屏
    sprite.pushSprite(0,0);
    
    for(int i=19;i>=1;i--){//因为要添加数据进来，所以除了第0位，从第1位开始所有数据后移一位
      symbol_array[i]=symbol_array[i-1];
    }
    symbol_array[0]=current_symbol;//每次新添加的都放到第0位
    if(current_symbol==1)code_str_raw+=";W1";//在遥控器代码的Button_funcs.cpp里找的到对应的传输协议
    else if(current_symbol==2)code_str_raw+=";W2";
    else if(current_symbol==3)code_str_raw+=";W3";
    else if(current_symbol==4)code_str_raw+=";W4";
    else if(current_symbol==5)code_str_raw+=";W5";
    else if(current_symbol==6)code_str_raw+=";W6";
    else if(current_symbol==7)code_str_raw+=";(2";//在遥控器代码的Card_process.cpp里找的到对应的传输协议，因为遥控器没有循环按钮、条件按钮这些
    else if(current_symbol==8)code_str_raw+=";(3";
    else if(current_symbol==9)code_str_raw+=";)";
    else if(current_symbol==10)code_str_raw+=";{";
    else if(current_symbol==11)code_str_raw+=";}";
    else if(current_symbol==12)code_str_raw+=";[";
    else if(current_symbol==13)code_str_raw+=";]";
    else if(current_symbol==14)code_str_raw+=";<";
    else if(current_symbol==15)code_str_raw+=";>";
    code_str_raw_item_counter++;
    Serial.println(code_str_raw);
    for(int i=0;i<18;i++){
      int temp_symbol=symbol_array[i];
      if(temp_symbol!=0&&i<4)DrawSymbol(30*i,50,temp_symbol);//显示symbolX.bmp
    }
    previous_symbol_counter=symbol_counter;
    receive_voice_condition=1;
    receive_voice_flag=true;
  }else if(current_symbol==0&&previous_symbol_counter!=symbol_counter&&symbol_counter>=1){
    //由于current_symbol只在3个地方赋值，一个是clear，一个是收到'F'，一个是delete；收到F时不可能为0，clear时counter是0，因此只有delete会触发这个情况
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);//数据该更新了，要刷新黑屏
    sprite.pushSprite(0,0);
    if(symbol_counter>=1){
      for(int i=0;i<19;i++){//数组有20个，但是我们最多只寸18个symbol，因此18，19位都是0，再怎么循环到i+1也不会超过数组上限
        symbol_array[i]=symbol_array[i+1];
      }
      code_str_raw = delete_last_item();
      code_str_raw_item_counter--;
    }
    for(int i=0;i<18;i++){
      int temp_symbol=symbol_array[i];
      if(temp_symbol!=0&&i<4)DrawSymbol(30*i,50,temp_symbol);
    }
    
    previous_symbol_counter=symbol_counter;
    receive_voice_condition=5;
    receive_voice_flag=true;
  }else if(current_symbol==0&&previous_symbol_counter!=symbol_counter&&symbol_counter<1){
      face_condition=1;
      symbol_counter=0;
      previous_symbol_counter=0;
      for(int i=0;i<20;i++)symbol_array[i]=0;
      code_str_raw="&";
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



void TFT_turn_angry(int start_index, int end_index)
{
  //example：2usual10 (1).bmp
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4TurnAngry" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}
void TFT_angry_blink(int start_index, int end_index)
{
  //example：2usual10 (1).bmp
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4AngryBlink" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}

void TFT_angry(){
  if(previous_face_condition!=3){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=3;
    TFT_turn_angry(1,15);
  }
  DrawBmp("/4TurnAngry15.bmp");
  int temp_facial_switch_to = int(random(1,200));
  if(temp_facial_switch_to==1){
    TFT_angry_blink(1,3);
  }
}

void TFT_turn_happy(int start_index, int end_index)
{
  //example：2usual10 (1).bmp
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4TurnHappy" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}
void TFT_happy_blink(int start_index, int end_index)
{
  //example：2usual10 (1).bmp
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4HappyBlink" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}
void TFT_happy(){
  if(previous_face_condition!=4){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=4;
    TFT_turn_happy(1,14);
  }
  DrawBmp("/4TurnHappy14.bmp");
  int temp_facial_switch_to = int(random(1,200));
  if(temp_facial_switch_to==1){
    TFT_happy_blink(1,7);
  }
}

void TFT_turn_sad(int start_index, int end_index)
{
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4TurnSad" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}
void TFT_sad_blink(int start_index, int end_index)
{
  for (int i = start_index; i <= end_index; i++)
  {
    String filename = "";
    filename = "/4SadBlink" + String(i) + ".bmp";
    DrawBmp(filename);
    vTaskDelay(5/portTICK_PERIOD_MS);
  }
}
void TFT_sad(){
  if(previous_face_condition!=4){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    previous_face_condition=4;
    TFT_turn_sad(1,11);
  }
  DrawBmp("/4TurnSad11.bmp");
  int temp_facial_switch_to = int(random(1,200));
  if(temp_facial_switch_to==1){
    TFT_sad_blink(1,7);
  }
}

void TFT_usualExpression()
{
  if(previous_face_condition!=0){
    sprite.drawXBitmap(2, 2, black_background, 128, 128, TFT_BLACK, TFT_BLACK);
    sprite.pushSprite(0,0);
    if(previous_face_condition==3){//之前是生气
      TFT_angry_blink(1,2);//到眨眼合上眼
    }else if(previous_face_condition==4){//之前是开心
      TFT_happy_blink(1,4);//到眨眼合上眼
    }else if(previous_face_condition==4){//之前是难过
      TFT_sad_blink(1,4);//到眨眼合上眼
    }
    TFT_usual("01",1,7);
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

