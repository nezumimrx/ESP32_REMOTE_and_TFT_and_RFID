#include <Preferences.h>
extern boolean receive_data_flag;
extern boolean receive_voice_flag;
extern int receive_wheel_condition;
extern int receive_voice_condition;
extern byte rfid_block_buffer[18];
extern boolean read_new_card;

extern boolean connected_with_controller;
extern boolean first_time_play_voice;

extern int motor_speed;
extern int full_speed;
extern int slow_speed;
extern int remote_mode_stepped_card_condition;//遥控模式下踩到的卡片种类 0-没有踩到卡，1-踩到加速卡，2踩到减速卡，3踩到掉头卡，4踩到混乱卡，5踩到停顿卡，99-胜利卡
extern int remote_mode_stepped_card_counter;//踩到卡的计时器，踩到开启，3秒后失效
extern boolean remote_running;
extern int volume;
extern Preferences pref;
extern boolean remote_or_code_mode;

extern int face_condition;
extern int current_symbol;
extern int symbol_counter;//用来记录当前显示的指令数量
extern int symbol_array[20];//用来储存所有应该被显示的指令
extern uint8_t symbol_add_or_delete;
extern String code_str_raw;
extern String code_str_clean;
extern int code_str_raw_item_counter;
extern int code_str_raw_item_max;

extern boolean has_condition_type1;//在存储code_str_condition_type1的同时，将has_condition_type1置为1，没有则为0
extern boolean receive_condition_type1;//在上位机也就是控制器接收到下位机也就是机器人发来的触发了条件1状况时，置位1，执行完毕置为0
extern String code_str_condition_type1;//存储type1符号表示的条件语句，type1 目前为{}中的语句

extern boolean has_condition_type2;
extern boolean receive_condition_type2;//在上位机也就是控制器接收到下位机也就是机器人发来的触发了条件1状况时，置位1，执行完毕置为0
extern String code_str_condition_type2;//存储type2符号表示的条件语句, type2 目前为[]中的语句

extern boolean has_condition_type3;
extern boolean receive_condition_type3;//在上位机也就是控制器接收到下位机也就是机器人发来的触发了条件1状况时，置位1，执行完毕置为0
extern String code_str_condition_type3;//存储type3符号表示的条件语句, type3 目前为||中的语句
extern boolean instant_stop;
extern boolean start_cypher;

extern void Code_Process_TASK(void *parameters);
extern TaskHandle_t Code_Process_Handle;
extern boolean if_local_process_code;
extern boolean robot_started;

extern boolean flash_emo_counter_handle;
extern int flash_emo_counter;//在编程模式下临时显示某个表情之后再恢复为箭头的时间
extern int flash_emo_previous_face_condition;//用来储存在切换为flash_emo之前是什么表情
