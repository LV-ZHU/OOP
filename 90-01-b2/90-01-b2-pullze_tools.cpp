/* 2452487 信安 胡中芃 */
#include <iostream>
#include <conio.h>
#include "../include/cmd_console_tools.h"
#include "90-01-b2-pullze.h"
using namespace std;

/***************************************************************************
  函数名称：提示继续
  功    能：提示本小题结束，请输入End继续...
  输入参数：无
  返 回 值：无
  说    明：第一次不提示"输入错误，请重新输入"
***************************************************************************/
void to_be_continued()
{
	cout << endl << "本小题结束，请输入End继续...";
	char buf[256];
	bool first = true;
	while (1) {
		int x, y;
		cct_getxy(x, y);
		cin.getline(buf, 256);
		// 判断前3个字符是否为End或end（不区分大小写）
		if ((buf[0] == 'E' || buf[0] == 'e') && (buf[1] == 'N' || buf[1] == 'n') && (buf[2] == 'D' || buf[2] == 'd'))
			break;
		else {
			if (!first)
				cout << "输入错误，请重新输入" << endl;
			cct_gotoxy(x, y); // 回到输入起始位置
			cout << "    ";    // 清除前4个字符
			cct_gotoxy(x, y); // 回到输入起始位置，因为下一次就会执行到输入命令，需要再放回到起始位置
			first = false;
		}
	}
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
}

/***************************************************************************
  函数名称：获取数织大小
  功    能：输入数组大小(5/10/15) : 固定提示
  输入参数：无
  返 回 值：room=col=row
  说    明：输入错误反复输入
***************************************************************************/
int get_room()
{
	cct_cls();
	int room;
	while (1) {
		cout << "请输入区域大小(5/10/15) : ";
		cin >> room;
		if (cin.fail()) {
			cin.clear();
			cin.ignore(65536, '\n');
		}
		else if (room != 5 && room != 10 && room != 15)
			continue;
		else
			break;
	}
	return room;
}

/***************************************************************************
  函数名称：检查答案
  功    能：检查数织结果是否正确（支持多解判定）
  输入参数：arr[15][15]：实际数组；user_arr[15][15]：用户数组；row，col：行列
  返 回 值：bool - true表示正确，false表示错误
  说    明：通过比较行列提示数字来判定，支持多解情况
***************************************************************************/
bool check_answer(char arr[15][15], char user_arr[15][15], int row, int col)
{
	// 计算原始数组的行列提示数字
	int original_row_groups[15][15] = { 0 }, original_row_group_count[15] = { 0 };
	int original_col_groups[15][15] = { 0 }, original_col_group_count[15] = { 0 };

	// 计算用户数组的行列提示数字
	int user_row_groups[15][15] = { 0 }, user_row_group_count[15] = { 0 };
	int user_col_groups[15][15] = { 0 }, user_col_group_count[15] = { 0 };

	// 计算原始数组的行提示
	for (int i = 0; i < row; i++) {
		get_groups(arr[i], col, original_row_groups[i], original_row_group_count[i]);
	}

	// 计算原始数组的列提示
	for (int j = 0; j < col; j++) {
		char tmp[15];
		for (int i = 0; i < row; i++)
			tmp[i] = arr[i][j];
		get_groups(tmp, row, original_col_groups[j], original_col_group_count[j]);
	}

	// 计算用户数组的行提示
	for (int i = 0; i < row; i++) {
		get_groups(user_arr[i], col, user_row_groups[i], user_row_group_count[i]);
	}

	// 计算用户数组的列提示
	for (int j = 0; j < col; j++) {
		char tmp[15];
		for (int i = 0; i < row; i++)
			tmp[i] = user_arr[i][j];
		get_groups(tmp, row, user_col_groups[j], user_col_group_count[j]);
	}

	// 比较行提示数字
	for (int i = 0; i < row; i++) {
		if (original_row_group_count[i] != user_row_group_count[i])
			return false;
		for (int g = 0; g < original_row_group_count[i]; g++) {
			if (original_row_groups[i][g] != user_row_groups[i][g])
				return false;
		}
	}

	// 比较列提示数字
	for (int j = 0; j < col; j++) {
		if (original_col_group_count[j] != user_col_group_count[j])
			return false;
		for (int g = 0; g < original_col_group_count[j]; g++) {
			if (original_col_groups[j][g] != user_col_groups[j][g])
				return false;
		}
	}

	return true;
}

/***************************************************************************
  函数名称：show_keycode_info
  功    能：显示键码信息，用于模式F和模式G
  输入参数：int x, int y - 显示位置坐标；int keycode1, keycode2 - 键码
  返 回 值：无
  说    明：提取公共的键码显示逻辑，减少代码重复
***************************************************************************/
void show_keycode_info(int x, int y, int keycode1, int keycode2)
{
	cct_gotoxy(x, y);
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
	cout << "[读到键码] ";

	if (keycode2) {
		// 根据keycode2的值输出对应的箭头方向
		switch (keycode2) {
			case KB_ARROW_UP:
				cct_gotoxy(x, y);
				cout << "[读到上箭头]        ";
				break;
			case KB_ARROW_DOWN:
				cct_gotoxy(x, y);
				cout << "[读到下箭头]        ";
				break;
			case KB_ARROW_LEFT:
				cct_gotoxy(x, y);
				cout << "[读到左箭头]        ";
				break;
			case KB_ARROW_RIGHT:
				cct_gotoxy(x, y);
				cout << "[读到右箭头]        ";
				break;
			default:
				cout << keycode1 << "/" << keycode2 << "     ";  // 其他特殊键仍显示键码
				break;
		}
	}
	else
		cout << keycode1 << "/0      ";
}

/***************************************************************************
  函数名称：output_something
  功    能：小彩蛋 - 纯文字版数织游戏技巧简单总结
  输入参数：无
  返 回 值：无
  说    明：显示数织游戏的基础方法和进阶技巧
***************************************************************************/
void output_something()
{
	cct_cls();
	cout << "一、基础方法：" << endl;
	cout << "1.寻找满行/满列，如果某行/列的提示数字总和加上间隔数(数字个数-1)正好等于总格子数，那么该行/列可完全确定。" << endl;
	cout << "2.overlap(抽屉原理)。优先确定在各种情况下一定是球的块，比如5*5数织提示一个3，那么球只能分布在123/234/345，故3一定是球" << endl;
	cout << "  可以参考视频https://b23.tv/f2mwvIW，讲了详细的操作方法" << endl;
	cout << "3.如果有靠边的行/列(row/col=1/数织大小)被确定，那么该数织难度会显著小于同大小的其他数织，因为靠墙方格都能确定" << endl;
	cout << "  同理利用overlap，如果接近边框有确定的球且该球所在的数字很大，则可以从墙边倒推确定更多格子" << endl;
	cout << "4.游玩时确定了某行/列一定要把剩下的画×(起到分隔作用)，数织较大时常常确定了中间某行列局面一分为多利于求解" << endl << endl;
	cout << "二、进阶技巧：" << endl;
	cout << "1.数字块的顺序不能变。当某行/列有多个数字块时，结合已知条件和顺序约束，有时能推出某个块只能出现在特定区间" << endl;
	cout << "2.行与列交叉验证。得到行的信息立刻看对应列，得到列的信息立刻看对应行。没有线索时逐行/列看有没有漏线索。" << endl;
	cout << "3.理论上球的个数多于一半必定线索足够，行列互相推进实在没线索可以像数独/扫雷一样假设关键位置进行推理" << endl;
	cout << "注：https://cn.puzzle-nonograms.com/网站上貌似并没有支持多解判定，可能是因为出现概率不高额外判定消耗太多算力" << endl;
	cout << "注：cmd窗口最好全屏否则选15*15大小的时候容易打印位置出现混乱" << endl << endl << endl;

	to_be_continued();
}
