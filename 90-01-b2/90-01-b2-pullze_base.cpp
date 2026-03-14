/* 2452487 信安 胡中芃 */
#include <iostream>
#include <iomanip>
#include <Windows.h>
#include "../include/cmd_console_tools.h"
#include "90-01-b2-pullze.h"
using namespace std;

/***************************************************************************
  函数名称：modeA
  功    能：模式A - 内部数组，原样输出
  输入参数：int& row - 行数，int& col - 列数（引用传递）
  返 回 值：无
  说    明：生成随机数织并以简单表格形式显示
***************************************************************************/
void modeA(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	cout << "初始数组：" << endl;
	show_array(arr, row, col, 'A');
	to_be_continued();
}

/***************************************************************************
  函数名称：modeB
  功    能：模式B - 内部数组，生成提示行列并输出
  输入参数：int& row - 行数，int& col - 列数（引用传递）
  返 回 值：无
  说    明：生成随机数织并显示带提示数字的表格
***************************************************************************/
void modeB(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	cout << "初始数组：" << endl;
	show_array(arr, row, col, 'B');
	to_be_continued();
}

/***************************************************************************
  函数名称：modeC
  功    能：模式C - 内部数组，游戏版
  输入参数：int& row - 行数，int& col - 列数（引用传递）
  返 回 值：无
  说    明：完整的文字版数织游戏，支持键盘输入、作弊模式、答案检查，while(1)最后cin才能一上来不默认提示输入错误
***************************************************************************/
void modeC(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	char user_arr[15][15];
	inner_array(row, col, arr);
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
			user_arr[i][j] = ' ';
	char buf[16] = { 0 };
	bool cheat_mode = false;

	cout << "初始数组：" << endl;
	show_array(arr, row, col, 'B', cheat_mode, user_arr);
	cin.ignore(65536, '\n');
	int sign_initial = 0;//防止第一次直接进入else分支提示输入错误

	while (1) {
		if ((buf[0] == 'X' || buf[0] == 'x') && buf[1] == '\0') {
			to_be_continued();
			return;
		}
		else if ((buf[0] == 'Y' || buf[0] == 'y') && buf[1] == '\0') {
			if (check_answer(arr, user_arr, row, col)) {
				cout << "提交成功，游戏结束" << endl;
				to_be_continued();
				return;
			}
			else
				cout << "提交错误, 可用作弊模式查看" << endl << endl;
		}
		else if ((buf[0] == 'Z' || buf[0] == 'z') && buf[1] == '\0') {
			cheat_mode = !cheat_mode;
			cout << "输入" << buf << "后:" << endl;
			show_array(arr, row, col, 'B', cheat_mode, user_arr);
		}

		else if (((buf[0] >= 'A' && buf[0] < 'A' + row) && (buf[1] >= 'a' && buf[1] < 'a' + row))
			&& buf[2] == '\0') {
			char row_ch = buf[0];
			char col_ch = buf[1];
			if (row_ch >= 'A' && row_ch < 'A' + row && col_ch >= 'a' && col_ch < 'a' + col) {
				int r = row_ch - 'A';
				int c = col_ch - 'a';
				if (r >= 0 && r < row && c >= 0 && c < col) {//点击互换操作
					if (user_arr[r][c] == 'O')
						user_arr[r][c] = ' ';
					else
						user_arr[r][c] = 'O';
				}
			}
			cout << "输入" << buf << "后:" << endl;
			show_array(arr, row, col, 'B', cheat_mode, user_arr);
		}
		else if (sign_initial != 0)
			cout << "输入错误" << endl << endl;
		else {
			cout << endl << endl;
			sign_initial = 1;//第一次不提示输入错误
		}
		cout << "命令形式：Aa=等价于图形游戏中鼠标左键选择Aa位(区分大小写)" << endl;
		cout << "             不需要支持图形界面的右键打叉，再次输入Aa相当于清除" << endl;
		cout << "          X/x=退出(新行仅有X/x，不分大小写)" << endl;
		cout << "          Y/y=提交(新行仅有Y/y，不分大小写)" << endl;
		cout << "          Z/z=作弊(新行仅有Z/z，不分大小写)" << endl;
		cout << "          Ctrl+c 强制退出" << endl;
		cout << "请输入 ：";
		cin.getline(buf, 16);
	}
}

/***************************************************************************
  函数名称：inner_array
  功    能：生成随机数织内部数组
  输入参数：int row - 行数，int col - 列数，char arr[15][15] - 数组
  返 回 值：无
  说    明：按要求生成一半格子上取整的数织，球数=(row*col)/2+1，类似扫雷随机生成
***************************************************************************/
void inner_array(int row, int col, char arr[15][15])
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
			arr[i][j] = 0;
	srand((unsigned int)time(0));
	int total_balls = (row * col) / 2 + 1;
	for (int num = 0; num < total_balls;) {
		int ROW = rand() % row;
		int COL = rand() % col;
		if (arr[ROW][COL] == 0) {
			arr[ROW][COL] = 'O';
			num++;
		}
	}
}

/***************************************************************************
  函数名称：print_jiajianA
  功    能：打印模式A的表格边框
  输入参数：int row - 行数，int col - 列数
  返 回 值：无
  说    明：打印简单的+-边框，每5列加一个分隔符
***************************************************************************/
void print_jiajianA(int row, int col)
{
	cout << "+-";
	for (int j = 0; j < col; j++) {
		if (j % 5 == 0)
			cout << "+-";
		cout << "--";
	}
	cout << "+" << endl;
}

/***************************************************************************
  函数名称：get_groups
  功    能：计算一行或一列的连续球群提示数字
  输入参数：const char* line - 一行或一列数据，int len - 长度，
			int* groups - 存储群组大小的数组，int& group_count - 群组数量
  返 回 值：无
  说    明：扫描连续的'O'字符，统计每个连续群组的大小
***************************************************************************/
void get_groups(const char* line, int len, int* groups, int& group_count)
{
	group_count = 0;
	int count = 0;
	for (int i = 0; i < len; i++) {
		if (line[i] == 'O')
			count++;
		else {
			if (count > 0) {
				groups[group_count++] = count;
				count = 0;
			}
		}
	}
	if (count > 0)
		groups[group_count++] = count;
}

/***************************************************************************
  函数名称：show_array
  功    能：显示数织数组，支持多种显示模式
  输入参数：char arr[15][15] - 答案数组，int row/col - 行列数，char sel - 模式选择，
			bool cheat_mode - 是否作弊模式，char user_arr[15][15] - 用户答案数组
  返 回 值：无
  说    明：根据sel参数决定显示方式：A-简单模式，B-带提示数字模式
***************************************************************************/
void show_array(char arr[15][15], int row, int col, char sel, bool cheat_mode, char user_arr[15][15])
{
	if (sel == 'A') {
		print_jiajianA(row, col);
		cout << "| ";
		for (int j = 0; j < col; j++) {
			if (j % 5 == 0)
				cout << "| ";
			cout << (char)('a' + j) << " ";
		}
		cout << "|" << endl;
		print_jiajianA(row, col);
		for (int i = 0; i < row; i++) {
			if (i > 0 && i % 5 == 0) {
				cout << "+-";
				for (int j = 0; j < col; j++) {
					if (j % 5 == 0)
						cout << "+-";
					cout << "--";
				}
				cout << "+" << endl;
			}
			cout << "|" << (char)('A' + i);
			for (int j = 0; j < col; j++) {
				if (j % 5 == 0)
					cout << "| ";
				cout << (arr[i][j] == 'O' ? 'O' : ' ') << " ";
			}
			cout << "|" << endl;
		}
		print_jiajianA(row, col);
		return;
	}
	else if (sel == 'B') {
		int max_row_group = 0, max_col_group = 0;
		int row_groups[15][15] = { 0 }, row_group_count[15] = { 0 };
		int col_groups[15][15] = { 0 }, col_group_count[15] = { 0 };
		for (int i = 0; i < row; i++) {
			get_groups(arr[i], col, row_groups[i], row_group_count[i]);
			if (row_group_count[i] > max_row_group)
				max_row_group = row_group_count[i];
		}
		for (int j = 0; j < col; j++) {
			char tmp[15];
			for (int i = 0; i < row; i++)
				tmp[i] = arr[i][j];
			get_groups(tmp, row, col_groups[j], col_group_count[j]);
			if (col_group_count[j] > max_col_group)
				max_col_group = col_group_count[j];
		}

		print_jiajianB(row, col, max_row_group, 0);

		for (int g = 0; g < max_col_group; g++) {
			for (int k = 0; k < max_row_group * 2 + 3; k++)
				cout << ' ';
			for (int j = 0; j < col; j++) {
				if (j % 5 == 0)
					cout << "| ";
				int blank = max_col_group - col_group_count[j];
				if (g < blank)
					cout << "  ";
				else
					cout << resetiosflags(ios::right) << setiosflags(ios::left) << setw(2) << col_groups[j][g - blank];
			}
			cout << "|" << endl;
		}

		print_jiajianB(row, col, max_row_group, 0);

		for (int k = 0; k < max_row_group * 2 + 3; k++)
			cout << ' ';
		cout << "| ";
		for (int j = 0; j < col; j++) {
			if (j % 5 == 0 && j != 0)
				cout << "| ";
			cout << (char)('a' + j) << " ";
		}
		cout << "|" << endl;

		print_jiajianB(row, col, max_row_group, 1);

		for (int i = 0; i < row; i++) {
			for (int g = 0; g < max_row_group - row_group_count[i]; g++)
				cout << "  ";
			for (int g = 0; g < row_group_count[i]; g++)
				cout << setiosflags(ios::right) << setw(2) << row_groups[i][g];
			cout << " |" << (char)('A' + i);
			for (int j = 0; j < col; j++) {
				if (j % 5 == 0)
					cout << "| ";
				if (cheat_mode && user_arr != NULL) {
					if (arr[i][j] == 'O' && user_arr[i][j] == 'O') {
						cct_setcolor(COLOR_HYELLOW, COLOR_BLUE);
						cout << "0";
						cct_setcolor(COLOR_BLACK, COLOR_WHITE);
						cout << " ";
					}
					else if (arr[i][j] == 'O' && user_arr[i][j] != 'O') {
						cct_setcolor(COLOR_BLACK, COLOR_WHITE);
						cout << "0 ";
					}
					else if (arr[i][j] != 'O' && user_arr[i][j] == 'O') {
						cct_setcolor(COLOR_HYELLOW, COLOR_BLUE);
						cout << "X";
						cct_setcolor(COLOR_BLACK, COLOR_WHITE);
						cout << " ";
					}
					else
						cout << "  ";
				}
				else if (user_arr != NULL) {
					if (user_arr[i][j] == 'O') {
						cct_setcolor(COLOR_HYELLOW, COLOR_BLUE);
						cout << "O";
						cct_setcolor(COLOR_BLACK, COLOR_WHITE);
						cout << " ";
					}
					else
						cout << "  ";
				}
				else
					cout << (arr[i][j] == 'O' ? 'O' : ' ') << " ";
			}
			cout << "|" << endl;
			if ((i + 1) % 5 == 0 && i + 1 < row)
				print_jiajianB(row, col, max_row_group, 1);
		}
		print_jiajianB(row, col, max_row_group, 1);
	}
}

/***************************************************************************
  函数名称：print_jiajianB
  功    能：打印模式B的表格边框
  输入参数：int row/col - 行列数，int max_row_group - 最大行群组数，
			bool sign_jiahao - 是否为带行标识的边框
  返 回 值：无
  说    明：打印带提示数字区域的复杂边框，支持两种不同的边框样式
***************************************************************************/
void print_jiajianB(int row, int col, int max_row_group, bool sign_jiahao)
{
	if (sign_jiahao) {
		for (int i = 0; i < 2 * max_row_group + 1; i++)
			cout << "-";
		print_jiajianA(row, col);
	}
	else {
		for (int i = 0; i < 2 * max_row_group + 3; i++)
			cout << "-";
		for (int j = 0; j < col; j++) {
			if (j % 5 == 0)
				cout << "+-";
			cout << "--";
		}
		cout << "+" << endl;
	}
}
