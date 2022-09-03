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

extern int face_condition;
extern int current_symbol;
extern int symbol_counter;//用来记录当前显示的指令数量
extern int symbol_array[20];//用来储存所有应该被显示的指令