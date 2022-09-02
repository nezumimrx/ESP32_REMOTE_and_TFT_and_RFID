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
extern int volume;
extern Preferences pref;

extern int face_condition;
extern int current_symbol;
extern int symbol_counter;//用来记录当前显示的指令数量
extern int symbol_array[20];//用来储存所有应该被显示的指令