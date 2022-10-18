#include <Arduino.h>
#include "global_vars.h"
#include <pwm.h>
String previous_ir_read = "";
boolean ir_counter_start;
int ir_counter;
int potential_movement = 0; // 0-停止 1-前进 2-后退 3-左平移 4-右平移 5-左转 6-右转
int confirmed_movement = 0; // 
int ir_confirm_time_max= 7;
int previous_movement=0;
int potential_counter=0;

void Sensor_ir_init()
{
    pinMode(34, INPUT);
    pinMode(35, INPUT);
    pinMode(36, INPUT);
    pinMode(39, INPUT);
}

String ir_read_add_to_str(String str, int pin)
{
    uint8_t read_data = digitalRead(pin);
    str += read_data;
    return str;
}
void Sensor_ir_read()
{
    String ir_read = "";
    ir_read = ir_read_add_to_str(ir_read, 34);
    ir_read = ir_read_add_to_str(ir_read, 35);
    ir_read = ir_read_add_to_str(ir_read, 36);
    ir_read = ir_read_add_to_str(ir_read, 39);

    if (ir_counter_start == false && confirmed_movement==0)
    { //还没触发计时统计
        if (previous_ir_read == "1111" && ir_read == "0111"){ //前传感器被遮挡
            ir_counter_start = true;
            ir_counter = 0;
            potential_movement = 2;
            potential_counter=0;
        }
        else if (previous_ir_read == "1111" && ir_read == "1011"){ //后传感器被遮挡
            //开始计时，在计时结束前如果没有检测到左或者右传感器被触发，则确认potential_movement
            ir_counter_start = true;
            ir_counter = 0;
            potential_movement = 1;
            potential_counter=0;
        }
        else if (previous_ir_read == "1111" && ir_read == "1101"){ //左传感器被遮挡
            ir_counter_start = true;
            ir_counter = 0;
            potential_movement = 4;
            potential_counter=0;
        }
        else if (previous_ir_read == "1111" && ir_read == "1110"){ //右传感器被遮挡
            ir_counter_start = true;
            ir_counter = 0;
            potential_movement = 3;
            potential_counter=0;
        }
        else if ((previous_ir_read == "0111" || previous_ir_read == "1011" || previous_ir_read == "1101" || previous_ir_read == "1110") && ir_read == "1111"){ //传感器恢复
            ir_counter=false;
            ir_counter=0;
            potential_movement=0;
            potential_counter=0;
        }
        }else if(ir_counter_start == true && confirmed_movement==0) {
            if(potential_movement==1&&ir_counter<=(ir_confirm_time_max-2)){//后传感器先被遮挡了且时间统计还未结束
                if(ir_read=="1101"){//在计时时捕捉到了左传感器被遮挡，则向右转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=6;
                }else if(ir_read=="1110"){//在计时时捕捉到了右传感器被遮挡，则向左转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=5;
                }else if(ir_read=="1011"){//在计时时捕捉到了后传感器被遮挡，计数
                    potential_counter++;//这个要实际测过才知道，按现在芯片的运行速度，大概在ir_confirm_time_max-1的这段时间能扫描4400次
                }
            }
            else if(potential_movement==1&&ir_counter==(ir_confirm_time_max-1)&&potential_counter>=3500){//当确实读取到次数超过3次时，才判定为有效手势
                confirmed_movement=1;
            }
        
            if(potential_movement==2&&ir_counter<=(ir_confirm_time_max-2)){//前传感器先被遮挡了且时间统计还未结束
                if(ir_read=="1101"){//在计时时捕捉到了左传感器被遮挡，则向左转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=5;
                }else if(ir_read=="1110"){//在计时时捕捉到了右传感器被遮挡，则向右转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=6;
                }else if(ir_read=="0111"){//在计时时捕捉到了前传感器被遮挡，计数
                    potential_counter++;//这个要实际测过才知道，按现在芯片的运行速度，大概在ir_confirm_time_max-1的这段时间能扫描4400次
                }
            }else if(potential_movement==2&&ir_counter==(ir_confirm_time_max-1)&&potential_counter>=3500){
                confirmed_movement=2;
            }

            if(potential_movement==3&&ir_counter<=(ir_confirm_time_max-2)){//右传感器先被遮挡了且时间统计还未结束
                if(ir_read=="0111"){//在计时时捕捉到了前传感器被遮挡，则向右转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=5;
                }else if(ir_read=="1011"){//在计时时捕捉到了后传感器被遮挡，则向左转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=6;
                }else if(ir_read=="1110"){
                    potential_counter++;//这个要实际测过才知道，按现在芯片的运行速度，大概在ir_confirm_time_max-1的这段时间能扫描4400次
                
                }
            }else if(potential_movement==3&&ir_counter==(ir_confirm_time_max-1)&&potential_counter>=3500){
                confirmed_movement=3;
            }

            if(potential_movement==4&&ir_counter<=(ir_confirm_time_max-2)){//左传感器先被遮挡了且时间统计还未结束
                if(ir_read=="0111"){//在计时时捕捉到了前传感器被遮挡，则向左转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=6;
                }else if(ir_read=="1011"){//在计时时捕捉到了后传感器被遮挡，则向右转
                    ir_counter_start=false;
                    ir_counter=0;
                    confirmed_movement=5;
                }else if(ir_read=="1101"){
                    potential_counter++;//这个要实际测过才知道，按现在芯片的运行速度，大概在ir_confirm_time_max-1的这段时间能扫描4400次
                }
            }else if(potential_movement==4&&ir_counter==(ir_confirm_time_max-1)&&potential_counter>=3500){
                confirmed_movement=4;
            }
        }else if(ir_counter_start == false && confirmed_movement!=0&&previous_movement!=0){
            if(previous_ir_read=="1111"&&ir_read=="1111"){
                confirmed_movement=0;
            }
        }



    previous_ir_read = ir_read;
}

void Sensor_ir_movement(){
    if(previous_movement!=0 && confirmed_movement==0){
        pwm_stop();
    }else if(previous_movement==0&&confirmed_movement==1){
        pwm_forward();
    }else if(previous_movement==0&&confirmed_movement==2){
        pwm_backward();
    }else if(previous_movement==0&&confirmed_movement==3){
        mecanum_left();
    }else if(previous_movement==0&&confirmed_movement==4){
        mecanum_right();
    }else if(previous_movement==0&&confirmed_movement==5){
        pwm_rotate_left();
        delay(300);
    }else if(previous_movement==0&&confirmed_movement==6){
        pwm_rotate_right();
        delay(300);
    }
    previous_movement=confirmed_movement;
}