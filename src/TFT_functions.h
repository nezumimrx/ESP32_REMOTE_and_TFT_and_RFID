#ifndef _TFT_functions_H_
#define _TFT_functions_H_
#include <Arduino.h>

void TFT_func_init();
void TFT_func_draw(int start_index, int end_index);
void TFT_func_draw_blinkeye();
void TFT_func_draw_lookleftandright();
void TFT_drawArrow();
void TFT_waitforcode();
void TFT_usualExpression();
void TFT_angry();
void TFT_happy();
void TFT_sad();
void TFT_venture();
void TFT_win();
void TFT_points();
void TFT_noFuel();
void TFT_noTime();
void TFT_DrawString(String str, int x, int y);
void TFT_warning();
void TFT_stage_clear();
void TFT_switchFace(int num);
void DrawBmp(String filename);

#endif