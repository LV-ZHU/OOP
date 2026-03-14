/* 2452487 信安 胡中芃 */
#pragma once

/* 将所有const常量定义的头文件包含进来 */
#include "90-01-b1-hanoi_const_value.h"
#include "../include/simple_menu.h"


/* ------------------------------------------------------------------------------------------------------

	 本文件功能：
	1、为了保证 hanoi_main.cpp/hanoi_menu.cpp/hanoi_multiple_solutions.cpp 能相互访问函数的函数声明
	2、一个以上的cpp中用到的宏定义（#define）或全局只读（const）变量，个数不限
	3、可以参考 cmd_console_tools.h 的写法（认真阅读并体会）
   ------------------------------------------------------------------------------------------------------ */

void delay_step(int delay);//延时
void prompt(int* n, char* src, char* dst, char* tmp, int sel);//唯一一个允许指针的函数，输入提示
// 其他功能：数组初始化；步骤数初始化；菜单7偶数层的时候提前交换中间柱和目标柱
void hanoi(int n, char src, char tmp, char dst, char sel);//唯一一个核心递归函数
void move(int n, char src, char dst, char sel); //执行不同菜单功能的一级总函数
void henxiang(int n, char src, char dst, char sel, int loop);//横向移动(二级函数)
void print_horizontal_array();//横向数组打印（核心）
void zongxiang(int n, char src, char dst, char sel, int loop);//纵向移动函数，通过参数实现不同位置打印
void print_column(int n, char src, char dst, char sel);//画柱子
void move_animate(int n, char src, char dst, char sel);//动画打印
void mode_9(int n, char src, char dst);//模式9

extern int buzhoushu;            // 总移动步数
extern int towers[3][MAX_LAYER]; // A,B,C三柱现有圆盘的编号
extern int top[3];               // A,B,C三柱现有圆盘的数量
extern int delay;                // 延时
