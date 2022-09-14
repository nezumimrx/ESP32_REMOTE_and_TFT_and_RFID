#include "Code_parse.h"
#include <Arduino.h>
#include "global_vars.h"
#include "ESPNOW_SLAVE.h"
#include <pwm.h>
String looptimes_strFormat = ""; //这个looptimes_strFormat由于放在下面开空间的话会出现存不进去数据的情况，所以要放到全局变量中
boolean processing_condition=false;//当已经在运行一个条件语句程序时，不要踩到再运行一次了

void code_parse_emergent_stop(){
    instant_stop=1;
    vTaskSuspend(Code_Process_Handle);//先把线程停下来
    code_str_clean="";//清空执行的程序
    vTaskResume(Code_Process_Handle);//由于code_str_clean清空以及instant_stop置为1，因此小车必然停下来，再开启线程，空跑完
    pwm_stop();//然后把车停下来
    vTaskDelay(100/portTICK_PERIOD_MS);
    instant_stop=0;
}

void process_wheel(String str, int i)
{ // L0-stop L1-forward L2-BACK L3-LEFT L4-RIGHT L5-MECLEFT L6-MECRIGHT 
  // L001 L101 L201 01表示时间
  int move_condition=str.substring(i+1,i+2).toInt();
  char next_next_char = str[i + 2];
  //第一种情况，如果后面带上数字卡片表示时间了，例如前进卡片W1 加上数字卡片02 行程了W102的情况，那么判断next_next_char是0成立的话，就执行下面的运算
  if(next_next_char=='0'){
    int delay_time=str.substring(i+2,i+4).toInt();
    //send_data_now('W',move_condition);
    pwm_local_process_code_behaviors(move_condition);
    vTaskDelay((500*delay_time)/portTICK_PERIOD_MS);
    //delay(500*delay_time);
    pwm_local_process_code_behaviors(0);
    vTaskDelay(200/portTICK_PERIOD_MS);
    //delay(200);
    i=i+3;
  }
  //如果后面没有时间，只是单纯的W1\W2\W3\W4\W5\W6的话，停顿1秒
  else if(next_next_char!='0'){
    pwm_local_process_code_behaviors(move_condition);
    //delay(500);
    vTaskDelay(500/portTICK_PERIOD_MS);
    pwm_local_process_code_behaviors(0);
    //delay(200);
    vTaskDelay(200/portTICK_PERIOD_MS);
    i++;
  }

}

boolean process_D(char next_char, char next_next_char, int i)
{

  if (next_char <= 57 && next_char >= 48)
  {
    int next_char_int = next_char - '0';
    int next_next_char_int = next_next_char - '0';
    int delay_time = next_char_int * 10 + next_next_char_int;
    Serial.print(F("delay: "));
    Serial.println(delay_time);
    vTaskDelay((delay_time * 1000)/portTICK_PERIOD_MS);
    //delay(delay_time * 1000);
    return 1;
  }
  else
  {
    Serial.println("default delay 1 ");
    //delay(1000);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    return 0;
  }
}

void code_parse(String str)
{
  // Serial.println(str);
  int str_length = str.length(); //这里原来为什么是str.length()-1？？
  // Serial.print("str length: ");Serial.println(str_length);
  //////////////////////////////////////////////首先判断这个字符串首字母是不是0-9的数字，如果是0-9的数字，那就是循环指令，如果是A-z的字母，就不是循环
  int header = int(str[0]);
  int looptimes = 1; //

  if (header <= 57 && header >= 48)
  { //首字母是数字开头所以是循环
    looptimes_strFormat = "";
    for (int i = 0; i < str_length; i++)
    {
      int current_ASCII_number = int(str[i]);
      if (current_ASCII_number <= 57 && current_ASCII_number >= 48)
      {
        looptimes_strFormat += (current_ASCII_number - '0');
        // Serial.print("looptimes_strFormat: ");Serial.println(looptimes_strFormat); //读取数据后发现如果looptimes_strFormat不是全局变量，会有+=添加不进去的情况
      }
      else if (current_ASCII_number >= 65||current_ASCII_number<=47)
      { //读到A-z的字符，结束读取数字部分
        break;
      }
    }
    looptimes = looptimes_strFormat.toInt(); //这是真正循环了多少次
    if (looptimes == 0)
      looptimes = 2; //当格式为&(0DD)的时候表示用户只放了loop卡片但是没有加任何数字卡片，组合成&(02DD)这种格式，所以默认循环2次
  }
  ////正式运行循环中的字符
  for (int j = 0; j < looptimes; j++)
  {
    for (int i = 0; i < str_length; i++)
    {
      char ch = str[i];
      if (instant_stop)
      {
        i = str_length;
        j = looptimes;
        break;
      }
      if (receive_condition_type1 == 1 && has_condition_type1 == 1 && instant_stop == 0&&processing_condition==false)
      {
        Serial.println("proceed condition type1");
        receive_condition_type1 = 0; //这里接收到的标志位要在code_parse之前清空是为了防止套娃
        //
        //send_data_now('W',0);//先让小车停下来说话
        receive_voice_flag=true;
        receive_voice_condition=15;//第一种环境条件触发
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
        processing_condition=true;
        code_parse(code_str_condition_type1);
        processing_condition=false;
        Serial.println("condition type1 complete");
      }else if(receive_condition_type1 == 1 && has_condition_type1 == 0 && instant_stop == 0 && (has_condition_type2!=0||has_condition_type3!=0)){
        Serial.println("no condition1 commands");
        receive_condition_type1 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=18;//触发了条件但是没有录入对应的条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
      }else if(receive_condition_type1 == 1 && has_condition_type1 == 0 && instant_stop == 0 && has_condition_type2==0 && has_condition_type3==0){
        Serial.println("no condition commands whatsoever");
        receive_condition_type1 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=19;//触发了条件但是没有录入任何条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
      }

      if (receive_condition_type2 == 1 && has_condition_type2 == 1 && instant_stop == 0 && processing_condition==false)
      {
        Serial.println("proceed conditiontype2");
        receive_condition_type2 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=16;//第2种环境条件触发
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
        processing_condition=true;
        code_parse(code_str_condition_type2);
        processing_condition=false;
        Serial.println("condition type2 complete");
      }else if(receive_condition_type2 == 1 && has_condition_type2 == 0 && instant_stop == 0 && (has_condition_type1!=0||has_condition_type3!=0)){
        Serial.println("no condition2 commands");
        receive_condition_type2 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=18;//触发了条件但是没有录入对应的条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
      }else if(receive_condition_type2 == 1 && has_condition_type2 == 0 && instant_stop == 0 && has_condition_type1==0 && has_condition_type3==0){
        Serial.println("no condition commands whatsoever");
        receive_condition_type2 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=19;//触发了条件但是没有录入任何条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
      }

      if (receive_condition_type3 == 1 && has_condition_type3 == 1 && instant_stop == 0&&processing_condition==false)
      {
        Serial.println("proceed conditiontype3");
        receive_condition_type3 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=17;//第一种环境条件触发
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
        processing_condition=true;
        code_parse(code_str_condition_type3);
        processing_condition=false;
        Serial.println("condition type3 complete");
      }else if(receive_condition_type3 == 1 && has_condition_type3 == 0 && instant_stop == 0 && (has_condition_type1!=0||has_condition_type2!=0)){
        Serial.println("no condition3 commands");
        receive_condition_type3 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=18;//触发了条件但是没有录入对应的条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
        //
      }else if(receive_condition_type3 == 1 && has_condition_type3 == 0 && instant_stop == 0 && has_condition_type1==0 && has_condition_type2==0){
        Serial.println("no condition commands whatsoever");
        receive_condition_type3 = 0;
        //
        //send_data_now('W',0);//先让小车停下来说话
        pwm_local_process_code_behaviors(0);
        receive_voice_flag=true;
        receive_voice_condition=19;//触发了条件但是没有录入任何条件指令
        vTaskDelay(4000/portTICK_PERIOD_MS);
        //delay(4000);//小车停下1.5s处理语音及表情
      }

      if(survive_step_on_card==true){
        Serial.println("Survive step on point card");
        pwm_local_process_code_behaviors(0);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        survive_step_on_card=false;
      }

      if (ch == '(')
      {
        String loopString = str.substring(i + 1); //把遇到循环起始字符之后的全部截下来
        //如果遇到了起始符'('，而且subString有终止字符')'，那么我们要继续循环分析的loopString中直到第一次遇到')'所在的位置就都要跳过，因此indexEnd要在i所在位置的基础上+loopString.indexOf(')')
        //但是当遇到起始字符'('，但没有遇到')'，说明没有再嵌套循环，因此跳过整句loopString的长度就可以 例如(4W2)(3W2) 截取到的loopString是4W2)(3W2),因为还有(3W2)这个循环在所以就不能跳过整个loopString
        //例如(2A(3B)C)(4D), loopString=2A(3B)C)(4D)  例如(4D) 例如&(2D02(2D01(3D))D04)
        int indexofNegativeBracket = loopString.indexOf(')'); //第一次出现)的位置 在2A(3B)C)(4D)中,indexofNegativeBracket=5 在4D)中 indexofNegativeBracket=2 2D02(2D01(3D))D04)
        int indexEndSignal = 0;
        if (indexofNegativeBracket == (loopString.length() - 1))
        { //例如出现了2D01(3D) 的情况，3D)的反括弧已经是最后一位了所以没办法再提取下一个negativeSubstring
          indexEndSignal = loopString.length();
        }
        else
        {
          String negativeSubstring = loopString.substring(indexofNegativeBracket + 1); // C)(4D)
          int nextPositiveIndex = negativeSubstring.indexOf('(');                      //在C)(4D)中nextPositiveIndex=2
          if (nextPositiveIndex > 0)
            indexEndSignal = indexofNegativeBracket + nextPositiveIndex; //对于2A(3B)C)(4D)就是4D前面的(位置
          else if (nextPositiveIndex < 0)
          {
            indexEndSignal = loopString.lastIndexOf(')');
          }
        }

        if (indexEndSignal > 0)
        {
          String nextLevelLoop = loopString.substring(0, indexEndSignal);
          Serial.println(nextLevelLoop);
          code_parse(nextLevelLoop);
          i = i + indexEndSignal;
        }
        else if (indexEndSignal < 0)
        {
          code_parse(loopString);
          i = i + loopString.length();
        }
      }
      else if (ch == 'W')
      { // Wheel
        process_wheel(str, i);
      }
      else if (ch == 'D')
      { // delay
        char next_char = str[i + 1];
        char next_next_char = str[i + 2];
        if (process_D(next_char, next_next_char, i))
          i = i + 2;
        else
          i = i;
      }
    }
  }
}


