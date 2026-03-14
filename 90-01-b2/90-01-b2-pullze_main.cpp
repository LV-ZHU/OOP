/* 2452487 信安 胡中芃 */
#include <iostream>
#include <conio.h>
#include "../include/cmd_console_tools.h"
#include "90-01-b2-pullze.h"
using namespace std;

/***************************************************************************
  函数名称：main
  功    能：程序主函数，控制整个数织游戏的流程
  输入参数：无
  返 回 值：程序结束状态码
  说    明：通过循环显示菜单，根据用户选择调用对应的模式函数
***************************************************************************/
int main()
{
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
	char sel;
	int row, col;
	int loop = 1;
	while (loop) { //该实现过程与4-b4很类似，引入loop变量调用菜单和对应功能，为0就退出循环
		const char* items[] = {
		"---------------------------------------------------------",
		" A.内部数组，原样输出" ,
		" B.内部数组，生成提示行列并输出", 
		" C.内部数组，游戏版",
		" --------------------------------------------------------" ,
		" D.n*n的框架(无分隔线)，原样输出",
		" E.n*n的框架(无分隔线)，含提示行列",
		" F.n*n的框架(无分隔线)，显示初始状态，鼠标移动可显示坐标",
		" G.cmd图形界面完整版(无分隔线)", 
		" --------------------------------------------------------", 
		" H.n*n的框架(有分隔线)，原样输出", 
		" I.n*n的框架(有分隔线)，含提示行列", 
		" J.n*n的框架(有分隔线)，显示初始状态，鼠标移动可显示坐标", 
		" K.cmd图形界面完整版(有分隔线)" ,
		" --------------------------------------------------------", 
	    " L.小彩蛋(纯文字版数织游戏技巧简单总结)", 
		" --------------------------------------------------------", 
		" Q.退出", 
		"---------------------------------------------------------", 
		"[请选择:] ",
		NULL
		};
		const char* choice = "abcdefghijklqABCDEFGHIJKLQ";
		sel = simple_menu(items, choice);
		cout << endl;
		cout << endl;
		cout << endl;
		switch (sel) {
			case 'A':
				modeA(row, col);
				break;
			case 'B':
				modeB(row, col);
				break;
			case 'C':
				modeC(row, col);
				break;
			case 'D':
				modeD(row, col);
				break;
			case 'E':
				modeE(row, col);
				break;
			case 'F':
				modeF(row, col);
				break;
			case 'G':
				modeG(row, col);
				break;
			case 'H':
				modeH(row, col);
				break;
			case 'I':
				modeI(row, col);
				break;
			case 'J':
				modeJ(row, col);
				break;
			case 'K':
				modeK(row, col);
				break;
			case 'L':
				output_something();
				break;
			case 'Q':
				loop = 0;
				continue;
			default:
				continue;
		}//end of switch
	}

	cct_gotoxy(0, 23);
	cout << "请按任意键继续. . .";
	int ret = _getch();//防止智能warning
	cout << endl;

	return 0;
}