#ifndef _pwm_H_
#define _pwm_H_
#include <Arduino.h>

void pwm_init(const int PWM_PIN_A1,const int PWM_PIN_A2,const int PWM_PIN_B1,const int PWM_PIN_B2,const int PWM_PIN_C1,const int PWM_PIN_C2,const int PWM_PIN_D1, const int PWM_PIN_D2);
void pwm_setduty(uint16_t pin, uint16_t duty);
void pwm_motion(int a1,int a2,int b1,int b2,int c1,int c2,int d1,int d2);
void pwm_forward();
void pwm_backward();
void pwm_rotate_left();
void pwm_rotate_right();
void mecanum_left();
void mecanum_right();
void pwm_stop();
void pwm_receive_esp_now_behaviors();
#endif




