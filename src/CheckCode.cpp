#include <Arduino.h>
#include "global_vars.h"
#include <ESPNOW_SLAVE.h>


String remove_delimiters(String str)
{
    String clean_str = "";
    int code_item_counter = 0;
    for (int i = 0; i < (str.length()); i++)
    {
        char c = str.charAt(i);
        if (c != ';')
        {
            clean_str += c;
        }
        else if (c == ';')
        {
            code_item_counter++;
        }
    }
    Serial.println(clean_str);
    Serial.print("counter: ");
    Serial.println(code_item_counter);
    if (code_item_counter > code_str_raw_item_max)
    {
        clean_str = "Oversize";
    }
    return clean_str;
}

int check_condition(char condition_symbol_start, char condition_symbol_end, String str)
{ // 0 没有找到符号，1找到符号并且存到对应的code_str_condition_type中并且has_condition_type标志被转为true，同时剔除掉在code_str_clean的部分，2语法错误
    int positive_bracket = str.indexOf(condition_symbol_start);
    int negative_bracket = str.indexOf(condition_symbol_end);

    if (positive_bracket == -1 && negative_bracket == -1)
    {

        if (condition_symbol_start == '{')
        {
            has_condition_type1 = false;
            Serial.println("no condition {whatsoever}.");
        }
        else if (condition_symbol_start == '[')
        {
            has_condition_type2 = false;
            Serial.println("no condition [whatsoever].");
        }
        else if (condition_symbol_start == '<')
        {
            has_condition_type3 = false;
            Serial.println("no condition <whatsoever>.");
        }
        return 0; // no condition symbol '{''}'
    }
    else if ((positive_bracket >= 0 && negative_bracket == -1) || (positive_bracket == -1 && negative_bracket >= 0))
    {
        Serial.println("incomplete condition sentence, wrong code");
        return 2; // only have one bracket, wrong code '{' no '}' or no '{' has '}'
    }
    else if (positive_bracket >= 0 && negative_bracket >= 0)
    {
        Serial.println("may be a good code lets proceed to check it.");
        int positve_bracket_counter = 0;
        int negative_bracket_counter = 0;
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == condition_symbol_start)
                positve_bracket_counter++;
            if (str[i] == condition_symbol_end)
                negative_bracket_counter++;
        }
        Serial.print("positive bracket counter: ");
        Serial.println(positve_bracket_counter);
        Serial.print("negative bracket counter: ");
        Serial.println(negative_bracket_counter);
        if (positve_bracket_counter != 1 || negative_bracket_counter != 1)
        {
            Serial.println("too many brackets, illegal code");
            return 2;
        }
        else
        {
            if (positive_bracket >= negative_bracket)
            {
                Serial.println("positive bracket goes infront of negative bracket, illegal code");
                return 2;
            }
            else
            {
                String temp_condition_str = str.substring(positive_bracket + 1, negative_bracket);
                Serial.print("temp_condition_str: ");
                Serial.println(temp_condition_str);
                //这里很重要！我们该把裁剪出来的条件语句放到哪个type下的全局变量存储？目前{}是type1，[]是type2,||是type3，后续还可以添加，记得改这里
                if (condition_symbol_start == '{')
                {
                    code_str_condition_type1 = temp_condition_str;
                    has_condition_type1 = true;
                }
                if (condition_symbol_start == '[')
                {
                    code_str_condition_type2 = temp_condition_str;
                    has_condition_type2 = true;
                }
                if (condition_symbol_start == '|')
                {
                    code_str_condition_type3 = temp_condition_str;
                    has_condition_type3 = true;
                }
                code_str_clean.remove(positive_bracket, (negative_bracket - positive_bracket + 1));
                Serial.print("cut out of condition code_str_clean: ");
                Serial.println(code_str_clean);
                return 1;
            }
        }

        return 1;
    }
    else
    {
        Serial.println("i dont know what the fuck just happened,this should not happen");
        return 2;
    }
}

int legal_test_simple(String str){ //检测语句是否正确特别是当有循环开始(2的时候有没有反括弧)
    int positive_bracket_counter = 0;
    int negetive_bracket_counter = 0;
    if (str.startsWith("Oversize"))
    {
        return 2;
    }
    else
    {
        if (str.length() < 2)
        {
            return 3; //还没有录入编程指令
        }
        else if (str.length() >= 2)
        {
            int bracket_counter = 0;
            for (int i = 0; i < (str.length()); i++)
            {
                char c = str.charAt(i);
                if (c == '(')
                {
                    bracket_counter++;
                    positive_bracket_counter++;
                }
                else if (c == ')')
                {
                    bracket_counter--;
                    negetive_bracket_counter++;
                }
                if (bracket_counter < 0)
                {
                    return 0;
                    break;
                }
            }
            if (positive_bracket_counter == negetive_bracket_counter)
            {
                return 1;
            }
            else
                return 0;
        }
        else
            return 0; //没有可能会发生这种情况，只是让计算机不报warning
    }
}


void check_code(String code_str_raw_local)
{
    Serial.println("CodingMode RUN/PLAY!");
    // code_str_raw_local="&(2D02(2D01(3D))D04){(2D03D02)}";//用来测试逻辑的，实际使用要注释掉
    // code_str_raw_local="&W102W203";//用来测试逻辑的，实际使用要注释掉
    code_str_clean = remove_delimiters(code_str_raw_local);//这里记得要添加code_str_clean全局变量
    int check_condition_type1_result = check_condition('{', '}', code_str_clean); // 0-没有type1的条件判断符号{}, 1-正确（详细说明看checkcondition功能，2语法错误
    int check_condition_type2_result = check_condition('[', ']', code_str_clean); // 0-没有type2的条件判断符号[], 1-正确（详细说明看checkcondition功能，2语法错误
    int check_condition_type3_result = check_condition('<', '>', code_str_clean); // 0-没有type3的条件判断符号||, 1-正确（详细说明看checkcondition功能，2语法错误
    int legal_result = 0;
    legal_result = legal_test_simple(code_str_clean);
    if (check_condition_type1_result == 1)
        legal_test_simple(code_str_condition_type1);
    else if (check_condition_type1_result == 2)
        legal_result = 0; //语法错误
    if (check_condition_type2_result == 1)
        legal_test_simple(code_str_condition_type2);
    else if (check_condition_type2_result == 2)
        legal_result = 0; //语法错误
    if (check_condition_type3_result == 1)
        legal_test_simple(code_str_condition_type3);
    else if (check_condition_type3_result == 2)
        legal_result = 0; //语法错误



    if (legal_result == 1)
    {
        Serial.println("legal code");
        // play voice run and play emo_ragerthat!
        flash_emo_counter_handle=true;//开启flash_emo显示时间的计时器
        flash_emo_counter=0;//flash_emo显示时间计时器清零
        flash_emo_previous_face_condition=2;//储存一下切换为flash_emo之前是什么表情
        vTaskSuspend(TFT_TASK_Handle);
        TFT_instant_stop=true;
        vTaskResume(TFT_TASK_Handle);
        
        receive_voice_flag = true;
        receive_voice_condition = 6; //运行、启动、开始执行编程指令
        if(mode_switch_condition==2&&survive_time_counter_start==false&&survive_start==true){survive_time_counter_start=true;receive_voice_condition=73;}//在生存模式下，如果第一次接收到了用户的指令，那么就开始5分钟倒计时 语音提示倒计时开始
        if(mode_switch_condition==2&&survive_time_counter_start==true&&survive_start==true&&survive_fuel>0){//如果在限定时间内，每执行一次程序燃料数-1
            survive_fuel--;  
            TFT_points_refresh=true;
        }else if(mode_switch_condition==2&&survive_time_counter_start==true&&survive_start==true&&survive_fuel<=0){//如果在限定时间内，但是燃料耗尽了
            survive_start=false;
            survive_fuel=0;
            TFT_points_refresh=true;
            face_condition=5;
            cannot_start_cypher=2;
            receive_voice_flag=true;
            receive_voice_condition=55;//燃料耗尽
        }
        if(face_condition!=5)face_condition=4; //虽然看起来多余，但是实际上是因为在生存模式下，燃料耗尽时再显示笑脸就不对了，所以优先判断survive_fuel，如果没油了，那么face_condition就是5sad，其他情况都时笑脸
        step_on_right_card_when_start_cypher=false;//防止运行完start_cypher发提示音打断故事语音的变量，每次运行start_cypher恢复成false
        delay(1000);
        motor_speed=slow_speed;
        delay(1000);
        //
        

        instant_stop = 0;
        if(cannot_start_cypher==0)start_cypher = 1;//可以解析编程指令时
        if(start_cypher==1){
            if(mode_switch_condition==2&&survive_start==true){
                vTaskSuspend(TFT_TASK_Handle);
                TFT_instant_stop=true;
                vTaskResume(TFT_TASK_Handle);
                face_condition=8;
                flash_emo_previous_face_condition=8;//储存一下切换为flash_emo之前是什么表情
            }
        }

        
        vTaskResume(Code_Process_Handle);
    }
    else if (legal_result == 0)
    {
        Serial.println("illegal code");
        // play voice illegal_code and play emo_question mark!
        flash_emo_counter_handle=true;//开启flash_emo显示时间的计时器
        flash_emo_counter=0;//flash_emo显示时间计时器清零
        flash_emo_previous_face_condition=2;//储存一下切换为flash_emo之前是什么表情
        vTaskSuspend(TFT_TASK_Handle);
        TFT_instant_stop=true;
        vTaskResume(TFT_TASK_Handle);
        face_condition=5;
        

        receive_voice_flag = true;
        receive_voice_condition = 7; //程序有错误
        //
    }
    else if (legal_result == 3)
    {
        Serial.println("empty code");
        // play voice empty_code and play emo_silence!
        flash_emo_counter_handle=true;//开启flash_emo显示时间的计时器
        flash_emo_counter=0;//flash_emo显示时间计时器清零
        flash_emo_previous_face_condition=2;//储存一下切换为flash_emo之前是什么表情
        vTaskSuspend(TFT_TASK_Handle);
        TFT_instant_stop=true;
        vTaskResume(TFT_TASK_Handle);
        face_condition=5;

        receive_voice_flag = true;
        receive_voice_condition = 8; //没有录入程序指令
        //
    }
    else if (legal_result == 2)
    {
        Serial.println("oversize code");
        // play voice oversize_code and play emo 一团乱!
        flash_emo_counter_handle=true;//开启flash_emo显示时间的计时器
        flash_emo_counter=0;//flash_emo显示时间计时器清零
        flash_emo_previous_face_condition=2;//储存一下切换为flash_emo之前是什么表情
        vTaskSuspend(TFT_TASK_Handle);
        TFT_instant_stop=true;
        vTaskResume(TFT_TASK_Handle);
        face_condition=5;

        receive_voice_flag = true;
        receive_voice_condition = 2; //超出上限
        //
    }
}