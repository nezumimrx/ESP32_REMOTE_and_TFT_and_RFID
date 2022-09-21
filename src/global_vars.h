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
extern TaskHandle_t TFT_TASK_Handle;
extern boolean TFT_instant_stop;

extern boolean if_local_process_code;
extern boolean robot_started;

extern boolean flash_emo_counter_handle;
extern int flash_emo_counter;//在编程模式下临时显示某个表情之后再恢复为箭头的时间
extern int flash_emo_previous_face_condition;//用来储存在切换为flash_emo之前是什么表情

extern int mode_switch_condition;////0-遥控模式, 1 -编程闯关模式，2-编程积分模式
//这是个神奇的变量，大部分时候运行完start_cypher都有提示音，而有一种情况不需要提示音，那就是在闯关模式下如果踩到了某关的通关卡正准备播放语音，此时，就不需要提示音了，这个变量改为true。而每次恢复是在checkcode完成时
extern boolean step_on_right_card_when_start_cypher;
extern int cannot_start_cypher;//0-正常解析 1-编程闯关造成的 2-生存模式造成的
extern uint8_t stage_num;

extern boolean survive_mode_intro;
extern boolean survive_time_counter_start;
extern boolean survive_start;//这个值的存在是为了保证当切换为mode_switch_condition==2时，用户在没有扫描1号卡片的时候不会受到燃料限制，因为燃料计算只有当扫描了1号卡片才会在check_code中开启
extern boolean survive_step_on_card;

extern int survive_time_limit;//这个可以改，生存模式可运行时间的最大值，目前是5分钟 300秒 6000tick
extern int survive_time_counter;
extern int survive_fuel;//生存模式燃料值
extern int survive_collected_points;//生存模式积分
extern boolean TFT_points_refresh;

extern boolean enviroment_loop_record_handle;//开启记录环境卡片中循环命令包含的内容
extern boolean enviroment_condition_record_handle;//开启记录环境卡片中条件命令包含的内容
extern boolean enviroment_counter_short_stop;//这个是为了条件或循环指令卡在记录指令时，暂时暂停计时
extern int enviroment_card_record_time;//放在循环或条件卡上之后，往前走多久就是及记录多久
extern int loop_recorded_cards[3];//用来记录所有存进去的指令，目前上限是3张动作卡
extern int loop_recorded_counter;//用来记录存了几个循环命令触发时要执行的指令
extern int condition_recorded_cards[3];//用来记录所有存进去的指令，目前上限是3张动作卡
extern int condition_recorded_counter;//用来记录存了几个条件命令触发时要执行的指令


