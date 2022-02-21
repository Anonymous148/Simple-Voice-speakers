#ifndef __PAINT_H
#define __PAINT_H

int lcd_fd;
int *plcd;  
int CW,CH; //记录图片的长宽
unsigned int COLOR[800][480];
int readpic(char *filename); //图片路径名称
int lcd_init();
void lcd_uninit();
void draw_white(int dx,int dy); //dx，dy为偏移值 绘制图片
void lcd_draw_point(unsigned int color, int x, int y);
void clear_screen(); //以color颜色清屏
#endif
