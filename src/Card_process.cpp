#include "Card_process.h"
#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include "global_vars.h"
#include <Code_parse.h>
#include <TFT_functions.h>
#include <pwm.h>
#include <Voice_funcs.h>
#include <rfid_funcs.h>
int previous_receive_wheel_condition = 0; //用来储存突发事件前小车的运行状态
uint8_t stage_num = 0;
boolean played_story_array[10] = {0};                 //是否已经播放过语音了
boolean stage_clear_array[10] = {0};                  //记录用户是否通关了
boolean step_on_right_card_when_start_cypher = false; //如果正在运行start_cypher那么得让设备马上停下来且不发声
int cannot_start_cypher = 0;                          //由0-可以解析，1-闯关模式无法解析，2-燃料耗尽 3-倒计时结束 需要通过某种手段恢复成0 例如在编程闯关模式下一旦掉入陷阱则需要从新拿到任务起始位置才行

boolean survive_mode_intro = false;
int survive_fuel = 1;             //生存模式燃料值
int survive_collected_points = 0; //生存模式积分
int survive_time_counter = 0;     //生存时间计时器
int survive_time_limit = 6000;    //默认6000 生存最大时间 50ms/tick 测试可以用40之类的降低测试时间
int survive_points_max = 20;      //所有卡片都收集齐了的分数值，目前设定为20分
boolean survive_time_counter_start = false;
boolean survive_clear_array[10] = {0}; //记录踩了哪些资源点
boolean survive_start = false;
boolean survive_step_on_card = false;               //是否踩到了卡片上，如果踩上了，暂停5秒播放语音。相当于code_parse中的条件语句触发
boolean enviroment_loop_record_handle = false;      //用来控制是否开启记录loop的handle
boolean enviroment_condition_record_handle = false; //用来控制是否开启记录condition的handle
boolean enviroment_counter_short_stop = false;      //这个是为了条件或循环指令卡在记录指令时，暂时暂停计时
int enviroment_card_record_time = 40;               //放在循环或条件卡上之后，往前走多久就是及记录多久，目前前进总时间是2秒
int loop_recorded_cards[3] = {0};                   //目前读取环境循环卡和条件卡的记录上限是3张卡片，不可嵌套, 1-前进。2-左转。3-右转。
int loop_recorded_counter=0;
int condition_recorded_cards[3] = {0};
int condition_recorded_counter=0;

boolean instant_stop_loop=false;



void enviroment_condition_cards(byte block_buffer[18])
{
    if (block_buffer[1] == 0x11)
    {
        Serial.println("step on condition card type 1");
        send_data_now('C', 1);
        if (start_cypher == 1 && instant_stop == 0)
        {
            receive_condition_type1 = true;
        }
    }
    else if (block_buffer[1] == 0x12)
    {
        Serial.println("step on condition card type 2");
        send_data_now('C', 2);
        if (start_cypher == 1 && instant_stop == 0)
        {
            receive_condition_type2 = true;
        }
    }
    else if (block_buffer[1] == 0x13)
    {
        Serial.println("step on condition card type 3");
        send_data_now('C', 3);
        if (start_cypher == 1 && instant_stop == 0)
        {
            receive_condition_type3 = true;
        }
    }
}

void forward_card_emoji()
{
    flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
    flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
    flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
    vTaskSuspend(TFT_TASK_Handle);
    TFT_instant_stop = true;
    vTaskResume(TFT_TASK_Handle);
    face_condition = 13;
    pwm_stop();
    delay(2500);
    // 
}

void left_card_emoji(){
    flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
    flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
    flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
    vTaskSuspend(TFT_TASK_Handle);
    TFT_instant_stop = true;
    vTaskResume(TFT_TASK_Handle);
    face_condition = 14;
    pwm_stop();
    delay(2500);
    // 
}

void right_card_emoji(){
    flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
    flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
    flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
    vTaskSuspend(TFT_TASK_Handle);
    TFT_instant_stop = true;
    vTaskResume(TFT_TASK_Handle);
    face_condition = 15;
    pwm_stop();
    delay(2500);
}

void enviroment_remotebuff_cards(byte block_buffer[18])
{ // 0-没有踩到卡，1-踩到加速卡，2踩到减速卡，3踩到掉头卡，4踩到混乱卡，5踩到停顿卡，99-胜利卡
    if (block_buffer[1] == 0x11)
    { //加速卡
        remote_mode_stepped_card_condition = 1;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x12)
    { //减速卡
        remote_mode_stepped_card_condition = 2;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x13)
    { //掉头卡
        remote_mode_stepped_card_condition = 3;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x14)
    { //混乱卡
        remote_mode_stepped_card_condition = 4;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x15)
    { //停顿卡
        remote_mode_stepped_card_condition = 5;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x01)
    { //胜利卡
        remote_mode_stepped_card_condition = 99;
        remote_mode_stepped_card_counter = 0;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (block_buffer[1] == 0x16 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 10) && enviroment_loop_record_handle == false) //(counter==0)是没踩到卡片的情况，counter>10是踩到卡片且距离上一次卡片间隔大于500ms的情况
    { //主动前进卡
        //停下2秒说话变表情
        instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
        receive_voice_flag = true;
        receive_voice_condition = 46;
        forward_card_emoji();
        remote_mode_stepped_card_condition = 6;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x16 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 4) && enviroment_loop_record_handle == true) //在记录循环指令时遇到了这张卡
    { //主动前进卡
        //停下2秒说话变表情
        Serial.println("in here");
        if (loop_recorded_counter < 2)
        {
            enviroment_counter_short_stop = true; //暂时停止计时
            play_voice(111);//是前进卡没错吧？语音 
            forward_card_emoji();
            //
            loop_recorded_cards[loop_recorded_counter] = 1;
            loop_recorded_counter++;
            enviroment_counter_short_stop = false; //重新开始计时
        }
        else if (loop_recorded_counter == 2)
        { //记录当前卡片的同时结束记录
            pwm_stop();
            play_voice(111);//播放是直行卡
            forward_card_emoji();
            receive_voice_flag = true;
            receive_voice_condition = 81; //播放结束记录语音
            delay(2000);
            //
            loop_recorded_cards[2] = 1;//1-代表前进
            loop_recorded_counter = 3;//记录上限是3个卡片
            remote_mode_stepped_card_counter = enviroment_card_record_time; //直接将时间跳到上限结束记录,这是结束记录的其中一种情况，且这种情况一定记录了3个指令
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (block_buffer[1] == 0x17 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 10) && enviroment_loop_record_handle == false)
    { //主动左转卡
        //停下2秒说话变表情
        instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
        receive_voice_flag = true;
        receive_voice_condition = 47;
        left_card_emoji();
        //
        remote_mode_stepped_card_condition = 7;
        remote_mode_stepped_card_counter = 0;
    }else if (block_buffer[1] == 0x17 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 4) && enviroment_loop_record_handle == true) //在记录循环指令时遇到了这张卡
    { ///主动左转卡
        //停下2秒说话变表情
        if (loop_recorded_counter < 2)
        {
            enviroment_counter_short_stop = true; //暂时停止计时
            play_voice(115);//是左转指令没错吧
            left_card_emoji();
            //
            loop_recorded_cards[loop_recorded_counter] = 2;//左转是2
            loop_recorded_counter++;
            enviroment_counter_short_stop = false; //重新开始计时
        }
        else if (loop_recorded_counter == 2)
        { //记录当前卡片的同时结束记录
            pwm_stop();
            play_voice(115);//播放是直行卡
            left_card_emoji();
            receive_voice_flag = true;
            receive_voice_condition = 81; //播放结束记录语音
            delay(2000);
            //
            loop_recorded_cards[2] = 2;//左转是2
            loop_recorded_counter = 3;//记录上限是3个卡片
            remote_mode_stepped_card_counter = enviroment_card_record_time; //直接将时间跳到上限结束记录,这是结束记录的其中一种情况，且这种情况一定记录了3个指令
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (block_buffer[1] == 0x18 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 10) && enviroment_loop_record_handle == false) //没有在条件或循环指令卡记录过程中时
    { //主动右转卡
        //停下2秒说话变表情
        instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
        receive_voice_flag = true;
        receive_voice_condition = 48;
        right_card_emoji();
        //
        remote_mode_stepped_card_condition = 8;
        remote_mode_stepped_card_counter = 0;
    }else if (block_buffer[1] == 0x18 && (remote_mode_stepped_card_counter == 0 || remote_mode_stepped_card_counter > 4) && enviroment_loop_record_handle == true) //在记录循环指令时遇到了这张卡
    { ///主动右转卡
        //停下2秒说话变表情
        if (loop_recorded_counter < 2)
        {
            enviroment_counter_short_stop = true; //暂时停止计时
            play_voice(119);//是右转指令没错吧
            right_card_emoji();
            //
            loop_recorded_cards[loop_recorded_counter] = 3;//右转是3
            loop_recorded_counter++;
            enviroment_counter_short_stop = false; //重新开始计时
        }
        else if (loop_recorded_counter == 2)
        { //记录当前卡片的同时结束记录
            pwm_stop();
            play_voice(119);//播放是直行卡
            left_card_emoji();
            receive_voice_flag = true;
            receive_voice_condition = 81; //播放结束记录语音
            delay(2000);
            //
            loop_recorded_cards[2] = 3;//右转是3
            loop_recorded_counter = 3;//记录上限是3个卡片
            remote_mode_stepped_card_counter = enviroment_card_record_time; //直接将时间跳到上限结束记录,这是结束记录的其中一种情况，且这种情况一定记录了3个指令
        }
    }

    //这个是循环2次的卡 0x19
    else if (block_buffer[1] == 0x19 && enviroment_loop_record_handle == false && enviroment_condition_record_handle == false) 
    //踩到循环卡 分2种情况，1.如果是正在运行其他动作指令则执行循环记录的卡;2如果是没在运行则前进读取要记录的卡
    {
        if (remote_mode_stepped_card_condition == 0)
        { //没有正在运行任何卡片，开始前进记录需要循环的卡片，到达3秒钟或再次遇到则停止记录
            //停下2秒说话，开始记录循环指令语音
            for (int i = 0; i < 3; i++)loop_recorded_cards[i] = 0; //先清空记录的所有卡片重新记录
            loop_recorded_counter = 0;      //清空已经记录的卡片数量
            receive_voice_flag = true;
            receive_voice_condition = 80; //开始记录要循环指令语音
            pwm_stop();
            delay(3000);
            //

            enviroment_loop_record_handle = true; //开启记录标志位，会在倒计时enviroment_card_record_time结束或开启记录状态再次遇到循环卡结束
            loop_recorded_counter = 0;
            remote_mode_stepped_card_condition = 9;
            remote_mode_stepped_card_counter = 0;
        }
        else if ((remote_mode_stepped_card_condition == 6 || remote_mode_stepped_card_condition == 7 || remote_mode_stepped_card_condition == 8) && loop_recorded_counter > 0)
        { //正在运行某种动作指令时遇到了循环卡且循环指令不为空，则执行循环记录的卡
            //停下2秒说话变表情
            instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
            receive_voice_flag = true;
            receive_voice_condition = 83;          //开始执行2次循环指令
            flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
            flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
            flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
            vTaskSuspend(TFT_TASK_Handle);
            TFT_instant_stop = true;
            vTaskResume(TFT_TASK_Handle);
            face_condition = 11;
            pwm_stop();
            delay(2500);
            //
            for (int i = 0; i < 2; i++)
            { //这个是循环2次的卡
                if(i==0)play_voice(125);
                else if(i==1)play_voice(126);
                for (int j = 0; j < 3; j++)
                {
                    if (loop_recorded_cards[j] == 1)
                    { //前进
                        //停下2秒说话变表情
                        forward_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 6;
                        remote_mode_stepped_card_counter = 0;
                        //delay(2000); //每个动作大概耗时2秒，暂时这里是2秒
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 2)
                    { //左转
                        //停下2秒说话变表情
                        left_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 7;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 3)
                    { //右转
                        //停下2秒说话变表情
                        right_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 8;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }

                    if(instant_stop_loop==true)break;
                }
                if(instant_stop_loop==true)break;
            }
            if(instant_stop_loop==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令，打断之后，执行一次card_process
                card_process(rfid_block_buffer);
            }
        }
    }

    //这个是循环2次的卡 0x19
    else if (block_buffer[1] == 0x20 && enviroment_loop_record_handle == false && enviroment_condition_record_handle == false) 
    //踩到循环卡 分2种情况，1.如果是正在运行其他动作指令则执行循环记录的卡;2如果是没在运行则前进读取要记录的卡
    {
        if (remote_mode_stepped_card_condition == 0)
        { //没有正在运行任何卡片，开始前进记录需要循环的卡片，到达3秒钟或再次遇到则停止记录
            //停下2秒说话，开始记录循环指令语音
            for (int i = 0; i < 3; i++)loop_recorded_cards[i] = 0; //先清空记录的所有卡片重新记录
            loop_recorded_counter = 0;      //清空已经记录的卡片数量
            receive_voice_flag = true;
            receive_voice_condition = 80; //开始记录要循环指令语音
            pwm_stop();
            delay(3000);
            //

            enviroment_loop_record_handle = true; //开启记录标志位，会在倒计时enviroment_card_record_time结束或开启记录状态再次遇到循环卡结束
            loop_recorded_counter = 0;
            remote_mode_stepped_card_condition = 9;
            remote_mode_stepped_card_counter = 0;
        }
        else if ((remote_mode_stepped_card_condition == 6 || remote_mode_stepped_card_condition == 7 || remote_mode_stepped_card_condition == 8) && loop_recorded_counter > 0)
        { //正在运行某种动作指令时遇到了循环卡且循环指令不为空，则执行循环记录的卡
            //停下2秒说话变表情
            instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
            receive_voice_flag = true;
            receive_voice_condition = 84;          //开始执行3次循环指令
            flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
            flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
            flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
            vTaskSuspend(TFT_TASK_Handle);
            TFT_instant_stop = true;
            vTaskResume(TFT_TASK_Handle);
            face_condition = 11;
            pwm_stop();
            delay(2500);
            //
            for (int i = 0; i < 3; i++)
            { //这个是循环2次的卡
                if(i==0)play_voice(125);
                else if(i==1)play_voice(126);
                else if(i==2)play_voice(127);
                for (int j = 0; j < 3; j++)
                {
                    if (loop_recorded_cards[j] == 1)
                    { //前进
                        //停下2秒说话变表情
                        forward_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 6;
                        remote_mode_stepped_card_counter = 0;
                        //delay(2000); //每个动作大概耗时2秒，暂时这里是2秒
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 2)
                    { //左转
                        //停下2秒说话变表情
                        left_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 7;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 3)
                    { //右转
                        //停下2秒说话变表情
                        right_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 8;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }

                    if(instant_stop_loop==true)break;
                }
                if(instant_stop_loop==true)break;
            }
            if(instant_stop_loop==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令，打断之后，执行一次card_process
                card_process(rfid_block_buffer);
            }
        }
    }

    else if (block_buffer[1] == 0x31 && enviroment_loop_record_handle == false && enviroment_condition_record_handle == false) 
    //踩到条件卡 条件卡其实就是只运行一次的循环卡
    {
        if (remote_mode_stepped_card_condition == 0)
        { //没有正在运行任何卡片，开始前进记录需要条件运行的卡片，到达2秒钟或再次遇到则停止记录
            //停下2秒说话，开始记录条件指令语音
            for (int i = 0; i < 3; i++)loop_recorded_cards[i] = 0; //先清空记录的所有卡片重新记录
            loop_recorded_counter = 0;      //清空已经记录的卡片数量
            receive_voice_flag = true;
            receive_voice_condition = 85; //开始记录条件指令
            pwm_stop();
            delay(3000);
            //

            enviroment_loop_record_handle = true; //开启记录标志位，会在倒计时enviroment_card_record_time结束或开启记录状态再次遇到循环卡结束
            loop_recorded_counter = 0;
            remote_mode_stepped_card_condition = 9;
            remote_mode_stepped_card_counter = 0;
        }
        else if ((remote_mode_stepped_card_condition == 6 || remote_mode_stepped_card_condition == 7 || remote_mode_stepped_card_condition == 8) && loop_recorded_counter > 0)
        { //正在运行某种动作指令时遇到了条件卡且要运行的指令不为空，则执行条件记录的卡
            //停下2秒说话变表情
            instant_stop_loop=false; //默认不停止循环，只有当等待动作完成时再次扫到新卡则停止循环
            receive_voice_flag = true;
            receive_voice_condition = 88;          //开始执行条件命令
            flash_emo_counter_handle = true;       //开启flash_emo显示时间的计时器
            flash_emo_counter = 0;                 // flash_emo显示时间计时器清零
            flash_emo_previous_face_condition = 0; //储存一下切换为flash_emo之前是什么表情
            vTaskSuspend(TFT_TASK_Handle);
            TFT_instant_stop = true;
            vTaskResume(TFT_TASK_Handle);
            face_condition = 11;
            pwm_stop();
            delay(2500);
            //
            for (int i = 0; i < 1; i++)
            { //这个是循环2次的卡
                for (int j = 0; j < 3; j++)
                {
                    if (loop_recorded_cards[j] == 1)
                    { //前进
                        //停下2秒说话变表情
                        forward_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 6;
                        remote_mode_stepped_card_counter = 0;
                        //delay(2000); //每个动作大概耗时2秒，暂时这里是2秒
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 2)
                    { //左转
                        //停下2秒说话变表情
                        left_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 7;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }
                    else if (loop_recorded_cards[j] == 3)
                    { //右转
                        //停下2秒说话变表情
                        right_card_emoji();
                        //
                        remote_mode_stepped_card_condition = 8;
                        remote_mode_stepped_card_counter = 0;
                        for(int z =0;z<42;z++){//替换delay2000，用这段时间不断扫描卡，这个42次是试出来的，差不多2秒
                            if(rfid_scan_card()==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令
                                instant_stop_loop=true;
                                break;
                            }
                            delay(2);
                        }
                    }

                    if(instant_stop_loop==true)break;
                }
                if(instant_stop_loop==true)break;
            }
            if(instant_stop_loop==true){//如果在等待的这段时间，也就是机器在走动的时候碰到了新的卡，则通过instant_stop_loop打断当前循环的命令，打断之后，执行一次card_process
                card_process(rfid_block_buffer);
            }
        }
    }
}

void process_mission(int current_stage, int current_stage_voice_condition)
{
    cannot_start_cypher = 0;
    if (stage_clear_array[current_stage - 1] == 0)
    {                                                   //如果第上一关任务未完成且第当前关任务已经扫描过
        if (played_story_array[current_stage - 1] == 1) //本关的任务已经开始了
        {
            stage_clear_array[current_stage - 1] = 1; //通关啦！
            receive_voice_flag = true;
            receive_voice_condition = current_stage_voice_condition; //播放第前一关通关故事以及第本关开始的故事
            played_story_array[current_stage] = 1;
            step_on_right_card_when_start_cypher = true; //防止运行完start_cypher发提示音打断故事语音

            TFT_switchFace(12);
        }
        else if (played_story_array[current_stage - 1] == 0) //你他妈连上一关都没开始
        {
            stage_clear_array[current_stage - 1] = 0;
            receive_voice_flag = true;
            receive_voice_condition = 52; //滚蛋，先把上一关任务开始了再说
        }
    }
    else if (stage_clear_array[current_stage - 1] == 1) //如果第上一关任务完成了
    {
        if (stage_clear_array[current_stage] == 0 && played_story_array[current_stage] == 1)
        { //第2关任务没完成呢,但是播放过语音

            receive_voice_flag = true;
            receive_voice_condition = 50;
        }
        else if (stage_clear_array[current_stage] == 1 && played_story_array[current_stage] == 1)
        { //请移步下一关任务点
            receive_voice_flag = true;
            receive_voice_condition = 51;
        }
    }
}

void enviroment_story_cards(byte block_buffer[18])
{ //什么时候扫描这些卡片生效？ 当start_cypher==0也就是没有执行编程指令的时候；另外当start_cypher==1 且执行当前任务时碰到下一关对应卡片，例如第3张卡片生效的情况是，执行第2关且第2关未通过，碰到第3关卡片，played_story_array[1]==1,stage_clear_array[1]==0
    if (block_buffer[1] == 0x11 && start_cypher == 0)
    { //第一关开始
        receive_voice_flag = true;
        cannot_start_cypher = 0;
        if (played_story_array[0] == 0)
            stage_clear_array[0] == 0; //如果没播放过这个卡片的故事，那么必然没有完成过这个任务
        //所以只剩下3种情况:1.没播过故事，任务未完成，2播放过故事、任务未完成，3播放过故事，且任务完成
        if (played_story_array[0] == 0 && stage_clear_array[0] == 0)
        { // 1.没播过故事，任务未完成
            receive_voice_condition = 31;
            stage_num = 0;

            played_story_array[0] = 1;
        }
        else if (played_story_array[0] == 1 && stage_clear_array[0] == 0)
        { // 2播放过故事、任务未完成
            receive_voice_condition = 50;
        }
        else if (played_story_array[0] == 1 && stage_clear_array[0] == 1)
        { // 3播放过故事，且任务完成
            receive_voice_condition = 51;
        }
    }
    //第1关结束，第2关开始, 如果已经开始start_cypher那么这张卡片生效的情况就只有执行第1关任务且未完成第1关任务时，因为如果已完成第1关任务再碰到这张卡就说明
    else if (block_buffer[1] == 0x12 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[0] == 1 && stage_clear_array[0] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 1;
            process_mission(stage_num, 32);
        } //未通关
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第2关结束，第3关开始
    else if (block_buffer[1] == 0x13 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[1] == 1 && stage_clear_array[1] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 2;
            process_mission(stage_num, 33);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第3关结束，第4关开始
    else if (block_buffer[1] == 0x14 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[2] == 1 && stage_clear_array[2] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 3;
            process_mission(stage_num, 34);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第4关结束，第5关开始
    else if (block_buffer[1] == 0x15 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[3] == 1 && stage_clear_array[3] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 4;
            process_mission(stage_num, 35);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第5关结束，第6关开始
    else if (block_buffer[1] == 0x16 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[4] == 1 && stage_clear_array[4] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 5;
            process_mission(stage_num, 36);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第6关结束，第7关开始
    else if (block_buffer[1] == 0x17 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[5] == 1 && stage_clear_array[5] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 6;
            process_mission(stage_num, 37);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第7关结束，第8关开始
    else if (block_buffer[1] == 0x18 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[6] == 1 && stage_clear_array[6] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 7;
            process_mission(stage_num, 38);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    //第8关结束，第9关开始
    else if (block_buffer[1] == 0x19 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[7] == 1 && stage_clear_array[7] == 0)))
    {
        if (stage_num != 9)
        {
            stage_num = 8;
            process_mission(stage_num, 39);
        }
        else if (stage_num == 9)
        {
            receive_voice_flag = true;
            receive_voice_condition = 53;
            TFT_switchFace(7);
        } //已通关
    }
    else if (block_buffer[1] == 0x20 && (start_cypher == 0 || (start_cypher == 1 && played_story_array[8] == 1 && stage_clear_array[8] == 0)))
    {
        cannot_start_cypher = 0;
        if (stage_clear_array[8] == 0)
        {                                   //如果第上一关任务未完成且第当前关任务已经扫描过
            if (played_story_array[8] == 1) //本关的任务已经开始了
            {
                stage_clear_array[8] = 1; //通关啦！
                stage_clear_array[9] = 1; //因为是最后一关，所以直接完成
                receive_voice_flag = true;
                receive_voice_condition = 40; //播放第前一关通关故事以及第本关开始的故事
                played_story_array[9] = 1;
                step_on_right_card_when_start_cypher = true; //防止运行完start_cypher发提示音打断故事语音
                stage_num = 9;
                TFT_switchFace(7);
            }
            else if (played_story_array[8] == 0) //你他妈连上一关都没开始
            {
                stage_clear_array[8] = 0;
                receive_voice_flag = true;
                receive_voice_condition = 52; //滚蛋，先把上一关任务开始了再说
            }
        }
        else if (stage_clear_array[8] == 1 && stage_clear_array[9] == 1 && played_story_array[9] == 1) //如果第上一关任务完成了
        {                                                                                              //报告指挥官，在你的带领下，我们已经完成了整个任务！可以切换其他任务模式！比如自定义模式或生存挑战模式等等！
            TFT_switchFace(7);
            receive_voice_flag = true;
            receive_voice_condition = 53;
        }
    }
    else if (block_buffer[1] == 0x01 && start_cypher == 1)
    { //在运行编程指令时遇到陷阱卡type1
        receive_voice_flag = true;
        receive_voice_condition = 41;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x02 && start_cypher == 1)
    { //在运行编程指令时遇到陷阱卡type2
        receive_voice_flag = true;
        receive_voice_condition = 42;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x03 && start_cypher == 1)
    { //在运行编程指令时遇到陷阱卡type3
        receive_voice_flag = true;
        receive_voice_condition = 43;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x04 && start_cypher == 1)
    { //在运行编程指令时遇到陷阱卡type4
        receive_voice_flag = true;
        receive_voice_condition = 44;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x05 && start_cypher == 1)
    { //在运行编程指令时遇到陷阱卡type5
        receive_voice_flag = true;
        receive_voice_condition = 45;
        cannot_start_cypher = 1;
    }
}

void process_survive_cards(int current_card, int point)
{
    survive_collected_points += point;
    survive_fuel += point;
    survive_clear_array[current_card] = 1;
    TFT_points_refresh = true;
    if (survive_collected_points >= survive_points_max)
    { //所有卡片都收集齐了
        face_condition = 7;
        survive_start = false;
        receive_voice_flag = true;
        receive_voice_condition = 74; //在规定时间、燃料限制内，收集了所有的资源
        survive_time_counter_start = false;
        survive_time_counter = 0;
        cannot_start_cypher = 4;
    }
    else
    {
        receive_voice_flag = true;
        receive_voice_condition = (74 + point); //因为点数是从1-5，而75对应的是1点分时，79对应的5点分数，所以是从74+point
        survive_step_on_card = true;
    }
}

void enviroment_point_cards(byte block_buffer[18])
{
    if (block_buffer[1] == 0x11 && start_cypher == 0)
    {
        if (survive_mode_intro == false)
        {
            receive_voice_flag = true;
            receive_voice_condition = 71;
            survive_mode_intro = true;
            //分数按理说只在第一次扫描的时候清空
            survive_collected_points = 0;
            for (int i = 0; i < 10; i++)
            {
                survive_clear_array[i] = 0;
            }
        }
        else if (survive_mode_intro == true)
        {
            receive_voice_flag = true;
            receive_voice_condition = 50;
        }
        //初始化
        survive_start = true;
        survive_clear_array[0] = 1; //第一张卡片已经扫描了
        if (survive_fuel <= 1)
            survive_fuel = 1;

        TFT_points_refresh = true;
        face_condition = 8;
        survive_time_counter = 0;
        cannot_start_cypher = 0; //初始化自动变成可执行编程
    }
    //      写着数字2的卡           正在运行指令        燃料大于等于0    在生存时间survive_time_limit以内   已经扫描过1号起始卡   没有读取过这个卡片
    else if (block_buffer[1] == 0x12 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[1] == 0)
    {
        process_survive_cards(1, 1);
    }
    else if (block_buffer[1] == 0x13 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[2] == 0)
    {
        process_survive_cards(2, 2);
    }
    else if (block_buffer[1] == 0x14 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[3] == 0)
    {
        process_survive_cards(3, 2);
    }
    else if (block_buffer[1] == 0x15 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[4] == 0)
    {
        process_survive_cards(4, 5);
    }
    else if (block_buffer[1] == 0x16 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[5] == 0)
    {
        process_survive_cards(5, 1);
    }
    else if (block_buffer[1] == 0x17 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[6] == 0)
    {
        process_survive_cards(6, 1);
    }
    else if (block_buffer[1] == 0x18 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[7] == 0)
    {
        process_survive_cards(7, 2);
    }
    else if (block_buffer[1] == 0x19 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[8] == 0)
    {
        process_survive_cards(8, 1);
    }
    else if (block_buffer[1] == 0x20 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true && survive_clear_array[9] == 0)
    {
        process_survive_cards(9, 5);
    }

    else if (block_buffer[1] == 0x01 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true)
    { //在运行编程指令时遇到陷阱卡type1
        cannot_start_cypher = 1;
        receive_voice_flag = true;
        receive_voice_condition = 41;
    }
    else if (block_buffer[1] == 0x02 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true)
    { //在运行编程指令时遇到陷阱卡type2
        receive_voice_flag = true;
        receive_voice_condition = 42;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x03 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true)
    { //在运行编程指令时遇到陷阱卡type3
        receive_voice_flag = true;
        receive_voice_condition = 43;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x04 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true)
    { //在运行编程指令时遇到陷阱卡type4
        receive_voice_flag = true;
        receive_voice_condition = 44;
        cannot_start_cypher = 1;
    }
    else if (block_buffer[1] == 0x05 && start_cypher == 1 && survive_fuel >= 0 && survive_time_counter_start == true && survive_start == true)
    { //在运行编程指令时遇到陷阱卡type5
        receive_voice_flag = true;
        receive_voice_condition = 45;
        cannot_start_cypher = 1;
    }
}

void card_process(byte block_buffer[18])
{
    if (read_new_card)
    {
        if (block_buffer[0] == 0xC1 && (mode_switch_condition == 1 || mode_switch_condition == 2))
        { // Enviroment CONDITION CARDS TYPE1 第一类条件卡片 对应{}中的指令
            enviroment_condition_cards(block_buffer);
        }
        else if (block_buffer[0] == 0xC2 && mode_switch_condition == 0)
        {
            enviroment_remotebuff_cards(block_buffer);
        }
        else if (block_buffer[0] == 0xC3 && (mode_switch_condition == 1 || mode_switch_condition == 2))
        {
            if (mode_switch_condition == 1)
            {
                enviroment_story_cards(block_buffer);
            }
            else if (mode_switch_condition == 2)
            {
                enviroment_point_cards(block_buffer);
            }
        }
        // Serial.println(code_str_raw);
        //发送指令已添加声音
        read_new_card = false;
        for (int i = 0; i < 18; i++)
        {
            rfid_block_buffer[i] = 0x00;
        }
    }
}
