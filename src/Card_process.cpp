#include "Card_process.h"
#include <Arduino.h>
#include <ESPNOW_SLAVE.h>
#include "global_vars.h"
#include <Code_parse.h>

int previous_receive_wheel_condition = 0; //用来储存突发事件前小车的运行状态
boolean played_story_array[10] = {0};     //是否已经播放过语音了
boolean stage_clear_array[10] = {0};      //记录用户是否通关了
boolean step_on_right_card_when_start_cypher=false;//如果正在运行start_cypher那么得让设备马上停下来且不发声
int cannot_start_cypher=0;//由0-可以解析，1-闯关模式无法解析，2-燃料耗尽 3-倒计时结束 需要通过某种手段恢复成0 例如在编程闯关模式下一旦掉入陷阱则需要从新拿到任务起始位置才行 

boolean survive_mode_intro=false;
int survive_fuel=1;//生存模式燃料值
int survive_collected_points=0;//生存模式积分
int survive_time_counter=0;//生存时间计时器
int survive_time_limit=6000;//默认6000 生存最大时间 50ms/tick 测试可以用40之类的降低测试时间
int survive_points_max=20;//所有卡片都收集齐了的分数值，目前设定为20分
boolean survive_time_counter_start=false;
boolean survive_clear_array[10]={0};//记录踩了哪些资源点
boolean survive_start=false;
boolean survive_step_on_card=false;//是否踩到了卡片上，如果踩上了，暂停5秒播放语音。相当于code_parse中的条件语句触发

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
    else if (block_buffer[1] == 0x16)
    { //主动前进卡
        remote_mode_stepped_card_condition = 6;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x17)
    { //主动左转卡
        remote_mode_stepped_card_condition = 7;
        remote_mode_stepped_card_counter = 0;
    }
    else if (block_buffer[1] == 0x18)
    { //主动右转卡
        remote_mode_stepped_card_condition = 8;
        remote_mode_stepped_card_counter = 0;
    }
}

void process_mission(int current_stage, int current_stage_voice_condition)
{   
    cannot_start_cypher=0;
    if (stage_clear_array[current_stage-1] == 0)
    { //如果第上一关任务未完成且第当前关任务已经扫描过
        if (played_story_array[current_stage-1] == 1)//本关的任务已经开始了
        { 
            stage_clear_array[current_stage-1] = 1; //通关啦！
            receive_voice_flag = true;
            receive_voice_condition = current_stage_voice_condition; //播放第前一关通关故事以及第本关开始的故事
            played_story_array[current_stage] = 1;
            step_on_right_card_when_start_cypher=true;//防止运行完start_cypher发提示音打断故事语音
            
        }
        else if (played_story_array[current_stage-1] == 0)//你他妈连上一关都没开始
        { 
            stage_clear_array[current_stage-1] = 0;
            receive_voice_flag = true;
            receive_voice_condition = 52; //滚蛋，先把上一关任务开始了再说
        }
    }
    else if (stage_clear_array[current_stage-1] == 1)//如果第上一关任务完成了
    { 
        if (stage_clear_array[current_stage] == 0 && played_story_array[current_stage] == 1)
        { //第2关任务没完成呢,但是播放过语音
            receive_voice_flag = true;
            receive_voice_condition = 50;
        }
        else if (stage_clear_array[current_stage] == 1 && played_story_array[current_stage] == 1)
        {//请移步下一关任务点
            receive_voice_flag = true;
            receive_voice_condition = 51;
        }
    }
}

void enviroment_story_cards(byte block_buffer[18])
{//什么时候扫描这些卡片生效？ 当start_cypher==0也就是没有执行编程指令的时候；另外当start_cypher==1 且执行当前任务时碰到下一关对应卡片，例如第3张卡片生效的情况是，执行第2关且第2关未通过，碰到第3关卡片，played_story_array[1]==1,stage_clear_array[1]==0
    if (block_buffer[1] == 0x11 && start_cypher==0)
    { //第一关开始
        receive_voice_flag = true;
        cannot_start_cypher=0;
        if (played_story_array[0] == 0)
            stage_clear_array[0] == 0; //如果没播放过这个卡片的故事，那么必然没有完成过这个任务
        //所以只剩下3种情况:1.没播过故事，任务未完成，2播放过故事、任务未完成，3播放过故事，且任务完成
        if (played_story_array[0] == 0 && stage_clear_array[0] == 0)
        { // 1.没播过故事，任务未完成
            receive_voice_condition = 31;
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
    else if (block_buffer[1] == 0x12 && (start_cypher==0 || (start_cypher==1&&played_story_array[0]==1&&stage_clear_array[0]==0)) ){ process_mission(1,32);}
    //第2关结束，第3关开始
    else if (block_buffer[1] == 0x13 && (start_cypher==0 || (start_cypher==1&&played_story_array[1]==1&&stage_clear_array[1]==0))){ process_mission(2,33);}
    //第3关结束，第4关开始
    else if(block_buffer[1] == 0x14 && (start_cypher==0 || (start_cypher==1&&played_story_array[2]==1&&stage_clear_array[2]==0))){process_mission(3,34);}
    //第4关结束，第5关开始
    else if(block_buffer[1] == 0x15 && (start_cypher==0 || (start_cypher==1&&played_story_array[3]==1&&stage_clear_array[3]==0))){process_mission(4,35);}
    //第5关结束，第6关开始
    else if(block_buffer[1] == 0x16 && (start_cypher==0 || (start_cypher==1&&played_story_array[4]==1&&stage_clear_array[4]==0))){process_mission(5,36);}
    //第6关结束，第7关开始
    else if(block_buffer[1] == 0x17 && (start_cypher==0 || (start_cypher==1&&played_story_array[5]==1&&stage_clear_array[5]==0))){process_mission(6,37);}
    //第7关结束，第8关开始
    else if(block_buffer[1] == 0x18 && (start_cypher==0 || (start_cypher==1&&played_story_array[6]==1&&stage_clear_array[6]==0))){process_mission(7,38);}
    //第8关结束，第9关开始
    else if(block_buffer[1] == 0x19 && (start_cypher==0 || (start_cypher==1&&played_story_array[7]==1&&stage_clear_array[7]==0))){process_mission(8,39);}
    else if(block_buffer[1] == 0x20 && (start_cypher==0 || (start_cypher==1&&played_story_array[8]==1&&stage_clear_array[8]==0))){
        cannot_start_cypher=0;
        if (stage_clear_array[8] == 0)
        { //如果第上一关任务未完成且第当前关任务已经扫描过
            if (played_story_array[8] == 1)//本关的任务已经开始了
            { 
                stage_clear_array[8] = 1; //通关啦！
                stage_clear_array[9] = 1;//因为是最后一关，所以直接完成
                receive_voice_flag = true;
                receive_voice_condition = 40; //播放第前一关通关故事以及第本关开始的故事
                played_story_array[9] = 1;
                step_on_right_card_when_start_cypher=true;//防止运行完start_cypher发提示音打断故事语音
            }
            else if (played_story_array[8] == 0)//你他妈连上一关都没开始
            { 
                stage_clear_array[8] = 0;
                receive_voice_flag = true;
                receive_voice_condition = 52; //滚蛋，先把上一关任务开始了再说
            }
        }
        else if (stage_clear_array[8] == 1&&stage_clear_array[9] == 1 && played_story_array[9] == 1)//如果第上一关任务完成了
        { //报告指挥官，在你的带领下，我们已经完成了整个任务！可以切换其他任务模式！比如自定义模式或生存挑战模式等等！
            receive_voice_flag = true;
            receive_voice_condition = 53;
        }
    }
    else if(block_buffer[1] == 0x01 && start_cypher==1){//在运行编程指令时遇到陷阱卡type1
        receive_voice_flag=true;
        receive_voice_condition=41;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1] == 0x02 && start_cypher==1){//在运行编程指令时遇到陷阱卡type2
        receive_voice_flag=true;
        receive_voice_condition=42;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1] == 0x03 && start_cypher==1){//在运行编程指令时遇到陷阱卡type3
        receive_voice_flag=true;
        receive_voice_condition=43;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1] == 0x04 && start_cypher==1){//在运行编程指令时遇到陷阱卡type4
        receive_voice_flag=true;
        receive_voice_condition=44;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1] == 0x05 && start_cypher==1){//在运行编程指令时遇到陷阱卡type5
        receive_voice_flag=true;
        receive_voice_condition=45;
        cannot_start_cypher=1;
    }
    
}


void process_survive_cards(int current_card, int point){
    survive_collected_points+=point;
    survive_fuel+=point;
    survive_clear_array[current_card]=1;
    TFT_points_refresh=true;
    if(survive_collected_points>=survive_points_max){//所有卡片都收集齐了
        face_condition=7;
        survive_start=false;
        receive_voice_flag=true;
        receive_voice_condition=74;//在规定时间、燃料限制内，收集了所有的资源
        survive_time_counter_start=false;
        survive_time_counter=0;
        cannot_start_cypher=4;
    }else{
        receive_voice_flag=true;
        receive_voice_condition=(74+point);//因为点数是从1-5，而75对应的是1点分时，79对应的5点分数，所以是从74+point
        survive_step_on_card=true;
    }
}


void enviroment_point_cards(byte block_buffer[18])
{
    if (block_buffer[1] == 0x11&&start_cypher==0)
    {
        if(survive_mode_intro==false){
            receive_voice_flag=true;
            receive_voice_condition=71;
            survive_mode_intro=true;
        }else if(survive_mode_intro==true){
            receive_voice_flag=true;
            receive_voice_condition=50;

        }
        //初始化
        survive_start=true;
        survive_clear_array[0]=1;//第一张卡片已经扫描了
        survive_fuel=1;
        TFT_points_refresh=true;
        survive_collected_points=0;
        survive_time_counter=0;
        cannot_start_cypher=0;//初始化自动变成可执行编程
        for(int i=0;i<10;i++){
            survive_clear_array[i]=0;
        }
    }
    //      写着数字2的卡           正在运行指令        燃料大于等于0    在生存时间survive_time_limit以内   已经扫描过1号起始卡   没有读取过这个卡片
    else if(block_buffer[1]==0x12&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[1]==0){
        process_survive_cards(1,1);
    }
    else if(block_buffer[1]==0x13&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[2]==0){
        process_survive_cards(2,2);
    }else if(block_buffer[1]==0x14&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[3]==0){
        process_survive_cards(3,2);
    }else if(block_buffer[1]==0x15&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[4]==0){
        process_survive_cards(4,5);
    }else if(block_buffer[1]==0x16&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[5]==0){
        process_survive_cards(5,1);
    }else if(block_buffer[1]==0x17&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[6]==0){
        process_survive_cards(6,1);
    }else if(block_buffer[1]==0x18&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[7]==0){
        process_survive_cards(7,2);
    }else if(block_buffer[1]==0x19&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[8]==0){
        process_survive_cards(8,1);
    }else if(block_buffer[1]==0x20&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true&&survive_clear_array[9]==0){
        process_survive_cards(9,5);
    }

    else if(block_buffer[1]==0x01&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true){
        cannot_start_cypher=1;
        receive_voice_flag=true;
        receive_voice_condition=41;
    }
    else if(block_buffer[1]==0x02&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true){
        receive_voice_flag=true;
        receive_voice_condition=42;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1]==0x03&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true){
        receive_voice_flag=true;
        receive_voice_condition=43;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1]==0x04&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true){
        receive_voice_flag=true;
        receive_voice_condition=44;
        cannot_start_cypher=1;
    }
    else if(block_buffer[1]==0x05&&start_cypher==1&&survive_fuel>=0&&survive_time_counter_start==true&&survive_start==true){
        receive_voice_flag=true;
        receive_voice_condition=45;
        cannot_start_cypher=1;
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
