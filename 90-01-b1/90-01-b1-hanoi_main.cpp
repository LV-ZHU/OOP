/* 2452487 信安 胡中芃 */
#include <iostream>
#include <conio.h>
#include "../include/cmd_hdc_tools.h"
#include "../include/cmd_console_tools.h"
#include "90-01-b1-hanoi.h"
#include "90-01-b1-hanoi_const_value.h"
using namespace std;

/* ----------------------------------------------------------------------------------

	 本文件功能：
	1、放main函数
	2、初始化屏幕
	3、调用菜单函数（hanoi_menu.cpp中）并返回选项
	4、根据选项调用菜单各项对应的执行函数（hanoi_multiple_solutions.cpp中）

	 本文件要求：
	1、不允许定义全局变量（含外部全局和静态全局，const及#define不在限制范围内）
	2、静态局部变量的数量不限制，但使用准则也是：少用、慎用、能不用尽量不用
	3、按需加入系统头文件、自定义头文件、命名空间等

   ----------------------------------------------------------------------------------- */

   /***************************************************************************
	 函数名称：
	 功    能：
	 输入参数：
	 返 回 值：
	 说    明：
   ***************************************************************************/
int main()
{
	/* 将这段复制到main的最前面 */
	cout << "请确认当前cmd窗口的大小为40行*120列以上，字体为新宋体/16，按C继续，Q退出" << endl;
	while (1) {
		char ch = _getch();
		if (ch == 'C' || ch == 'c')
			break;
		if (ch == 'Q' || ch == 'q')
			return 0;
	}

	/* 从这里继续你的程序 */

	char sel;
	int loop = 1;
	int n;
	char src, dst, tmp;
	while (loop) { //该实现过程与4-b4很类似，引入loop变量调用菜单和对应功能
		const char* items[] = {
		"---------------------------------" ,
		"1.基本解",
		"2.基本解(步数记录)",
		"3.内部数组显示(横向)" ,
		"4.内部数组显示(纵向+横向)",
		"5.图形解-预备-画三个圆柱" ,
		"6.图形解-预备-在起始柱上画n个盘子",
		"7.图形解-预备-第一次移动",
		"8.图形解-自动移动版本",
		"9.图形解-游戏版",
		"0.退出",
		 "---------------------------------",
		"[请选择:] ",
		NULL
		};
		const char* choice = "0123456789";
		sel = simple_menu(items,choice);
		cout << endl;
		cout << endl;
		cout << endl;
		if (sel != '5' && sel != '0')
			prompt(&n, &src, &dst, &tmp, sel);//先提示输入
		switch (sel) {
			case '1':
				hanoi(n, src, tmp, dst, sel);
				break;
			case '2':
				hanoi(n, src, tmp, dst, sel);
				break;
			case '3':
				hanoi(n, src, tmp, dst, sel);
				break;
			case '4':
				zongxiang(n, src, dst, sel, 0);
				henxiang(n, src, dst, sel, 0);
				hanoi(n, src, tmp, dst, sel);
				cct_gotoxy(Status_Line_X, Status_Line_Y);
				break;
			case '5':
				cct_cls();
				print_column(0, 'A', 'A', sel);//n=0防止弹窗越界
				break;
			case '6':
				print_column(n, src, dst, sel);
				break;
			case '7':
				print_column(n, src, dst, sel);
				hanoi(1, src, tmp, dst, sel);
				break;
			case '8':
				zongxiang(n, src, dst, sel, 0);
				henxiang(n, src, dst, sel, 0);
				print_column(n, src, dst, sel);
				hanoi(n, src, tmp, dst, sel);
				cct_gotoxy(Status_Line_X, Status_Line_Y);
				break;
			case '9':
				zongxiang(n, src, dst, '8', 0);
				henxiang(n, src, dst, '8', 0);
				print_column(n, src, dst, sel);
				mode_9(n, src, dst);
				break;
			case '0':
				loop = 0;
				continue;
			default:
				continue;
		}//end of switch

		cout << endl;
		cout << "按回车键继续";
		while (_getch() != '\r')
			;
	}

	cct_gotoxy(Status_Line_X, Status_Line_Y);
	return 0;
}