#ifndef _TFT_functions_H_
#define _TFT_functions_H_
#include <Arduino.h>

void TFT_func_init();
void TFT_func_draw(int start_index, int end_index);
void TFT_func_draw_blinkeye();
void TFT_func_draw_lookleftandright();

void DrawBmp(String filename);
#endif