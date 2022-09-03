#include <Arduino.h>
#include "pwm.h"
#include "global_vars.h"
/* PWM使用的是ledc模块功能实现的，一共有16个通道
 * LEDC Chan to Group/Channel/Timer Mapping
 ** ledc: 0  => Group: 0, Channel: 0, Timer: 0
 ** ledc: 1  => Group: 0, Channel: 1, Timer: 0
 ** ledc: 2  => Group: 0, Channel: 2, Timer: 1
 ** ledc: 3  => Group: 0, Channel: 3, Timer: 1
 ** ledc: 4  => Group: 0, Channel: 4, Timer: 2
 ** ledc: 5  => Group: 0, Channel: 5, Timer: 2
 ** ledc: 6  => Group: 0, Channel: 6, Timer: 3
 ** ledc: 7  => Group: 0, Channel: 7, Timer: 3
 ** ledc: 8  => Group: 1, Channel: 0, Timer: 0
 ** ledc: 9  => Group: 1, Channel: 1, Timer: 0
 ** ledc: 10 => Group: 1, Channel: 2, Timer: 1
 ** ledc: 11 => Group: 1, Channel: 3, Timer: 1
 ** ledc: 12 => Group: 1, Channel: 4, Timer: 2
 ** ledc: 13 => Group: 1, Channel: 5, Timer: 2
 ** ledc: 14 => Group: 1, Channel: 6, Timer: 3
 ** ledc: 15 => Group: 1, Channel: 7, Timer: 3
 */

const int pwmA1_channel = 2;
const int pwmA2_channel = 3;
const int pwmB1_channel = 10;
const int pwmB2_channel = 11;
const int pwmC1_channel = 4;
const int pwmC2_channel = 5;
const int pwmD1_channel = 12;
const int pwmD2_channel = 13;
int pwm_pin_A1, pwm_pin_A2, pwm_pin_B1, pwm_pin_B2, pwm_pin_C1, pwm_pin_C2, pwm_pin_D1, pwm_pin_D2;
const int pwm_freq = 1000; // 1000hz

// PWM分辨率，取值为 0-20 之间，这里填写为8，那么后面的ledcWrite
// 这个里面填写的pwm值就在 0 - 2的8次方 之间 也就是 0-255
uint8_t pwm_res = 8; //如果是10就是0-1024

void pwm_init(const int PWM_PIN_A1, const int PWM_PIN_A2, const int PWM_PIN_B1, const int PWM_PIN_B2, const int PWM_PIN_C1, const int PWM_PIN_C2, const int PWM_PIN_D1, const int PWM_PIN_D2)
{
    pwm_pin_A1 = PWM_PIN_A1; //把值传递给cpp里的变量，这样在用户设置pwm的时候只要比对用户输入的pin和存储的pin是否一致就可以配置ledcWrite（channel duty）了
    pwm_pin_A2 = PWM_PIN_A2;
    pwm_pin_B1 = PWM_PIN_B1;
    pwm_pin_B2 = PWM_PIN_B2;
    pwm_pin_C1 = PWM_PIN_C1;
    pwm_pin_C2 = PWM_PIN_C2;
    pwm_pin_D1 = PWM_PIN_D1;
    pwm_pin_D2 = PWM_PIN_D2;
    pinMode(pwm_pin_A1, OUTPUT);
    pinMode(pwm_pin_A2, OUTPUT);
    pinMode(pwm_pin_B1, OUTPUT);
    pinMode(pwm_pin_B2, OUTPUT);
    pinMode(pwm_pin_C1, OUTPUT);
    pinMode(pwm_pin_C2, OUTPUT);
    pinMode(pwm_pin_D1, OUTPUT);
    pinMode(pwm_pin_D2, OUTPUT);
    ledcSetup(pwmA1_channel, pwm_freq, pwm_res); //从上表可以看出通道2和通道3都用的是timer1，为了避开timer0，因此不选通道0和通道1
    ledcSetup(pwmA2_channel, pwm_freq, pwm_res); //从上表可以看出通道2和通道3都用的是timer1，为了避开timer0，因此不选通道0和通道1
    ledcSetup(pwmB1_channel, pwm_freq, pwm_res); //所有A\B口都用timer1产生
    ledcSetup(pwmB2_channel, pwm_freq, pwm_res); //所有A\B口都用timer1产生
    ledcSetup(pwmC1_channel, pwm_freq, pwm_res); // C\D口选的是timer2
    ledcSetup(pwmC2_channel, pwm_freq, pwm_res); // C\D口选的是timer2
    ledcSetup(pwmD1_channel, pwm_freq, pwm_res); // C\D口选的是timer2
    ledcSetup(pwmD2_channel, pwm_freq, pwm_res); // C\D口选的是timer2
    ledcAttachPin(pwm_pin_A1, 2);
    ledcAttachPin(pwm_pin_A2, 3);
    ledcAttachPin(pwm_pin_B1, 10);
    ledcAttachPin(pwm_pin_B2, 11);
    ledcAttachPin(pwm_pin_C1, 4);
    ledcAttachPin(pwm_pin_C2, 5);
    ledcAttachPin(pwm_pin_D1, 12);
    ledcAttachPin(pwm_pin_D2, 13);
}

void pwm_setduty(uint16_t pin, uint16_t duty)
{
    if (pin == pwm_pin_A1)
        ledcWrite(pwmA1_channel, duty);
    else if (pin == pwm_pin_A2)
        ledcWrite(pwmA2_channel, duty);
    else if (pin == pwm_pin_B1)
        ledcWrite(pwmB1_channel, duty);
    else if (pin == pwm_pin_B2)
        ledcWrite(pwmB2_channel, duty);
    else if (pin == pwm_pin_C1)
        ledcWrite(pwmC1_channel, duty);
    else if (pin == pwm_pin_C2)
        ledcWrite(pwmC2_channel, duty);
    else if (pin == pwm_pin_D1)
        ledcWrite(pwmD1_channel, duty);
    else if (pin == pwm_pin_D2)
        ledcWrite(pwmD2_channel, duty);
}

void pwm_motion(int a1, int a2, int b1, int b2, int c1, int c2, int d1, int d2) // a1a2-左前，b1b2-右前，c1c2-左后，d1d2-右后
{
    ledcWrite(pwmA1_channel, a1);
    ledcWrite(pwmA2_channel, a2);
    ledcWrite(pwmB1_channel, b1);
    ledcWrite(pwmB2_channel, b2);
    ledcWrite(pwmC1_channel, c1);
    ledcWrite(pwmC2_channel, c2);
    ledcWrite(pwmD1_channel, d1);
    ledcWrite(pwmD2_channel, d2);
}

void pwm_forward()
{
    pwm_motion(motor_speed, 0, motor_speed, 0, motor_speed, 0, motor_speed, 0);
    Serial.println("moving forward");
}
void pwm_backward()
{
    pwm_motion(0, motor_speed, 0, motor_speed, 0, motor_speed, 0, motor_speed);
    Serial.println("moving backward");
}
void pwm_rotate_left()
{
    pwm_motion(0, motor_speed, motor_speed, 0, 0, motor_speed, motor_speed, 0);
    Serial.println("turning left");
}
void pwm_rotate_right()
{
    if(motor_speed>=130)
        pwm_motion(motor_speed, 0, 0, motor_speed, motor_speed, 0, 0, motor_speed);
    else if(motor_speed<130)
        pwm_motion(125, 0, 0, 125, 125, 0, 0, 125);
    Serial.println("turning right");
}
void mecanum_left()
{
    if(motor_speed>=130)
        pwm_motion(0, motor_speed, motor_speed, 0, motor_speed, 0, 0, motor_speed);
    else if(motor_speed<130)
        pwm_motion(0, 125, 125, 0, 125, 0, 0, 125);
    Serial.println("mec move left");
}
void mecanum_right()
{
    pwm_motion(motor_speed, 0, 0, motor_speed, 0, motor_speed, motor_speed, 0);
    Serial.println("mec move right");
}


void pwm_stop()
{
    pwm_motion(0, 0, 0, 0, 0, 0, 0, 0);
    Serial.println("stop");
}

void pwm_receive_esp_now_behaviors()
{
    if (receive_data_flag == true)
    {
        if (receive_wheel_condition == 0)
            pwm_stop();
        else if (receive_wheel_condition == 1)
            pwm_forward();
        else if (receive_wheel_condition == 2)
            pwm_backward();
        else if (receive_wheel_condition == 3)
            pwm_rotate_left();
        else if (receive_wheel_condition == 4)
            pwm_rotate_right();
        else if (receive_wheel_condition == 5)
            mecanum_left();
        else if (receive_wheel_condition == 6)
            mecanum_right();
        
        receive_data_flag = false;
    }
}
