/* 2452487 信安 胡中芃 */
#include <iostream>
#include <iomanip>
#include <Windows.h>
#include "../include/cmd_console_tools.h"
#include "90-01-b2-pullze.h"
using namespace std;

const int modeD_a_X = 4;
const int modeD_a_Y = 1;
const int modeH_a_X = 6;
const int modeH_a_Y = 1;

// 通用数据区域绘制函数，D和E模式完全一致的数据区域部分
void draw_data_area(int start_x, int start_y, int row, int col, char arr[15][15], char sel)
{
	if (sel == 'D') {
		// D模式显示列标识和行标识
		cct_gotoxy(start_x, start_y);
		for (int j = 0; j < col; j++) {
			cout << (char)('a' + j) << " ";
		}
		for (int i = 0; i < row; i++) {
			cct_gotoxy(0, start_y + 2 + i);
			cout << (char)('A' + i);
		}
	}
	else if (sel == 'E') {
		// E模式只显示列标识（行标识在左侧区域已处理）
		cct_gotoxy(start_x, start_y);
		for (int j = 0; j < col; j++) {
			cout << (char)('a' + j) << " ";
		}
	}
	// 显示顶部框线
	cct_gotoxy(start_x - 2, start_y + 1);
	cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
	cout << "╔";
	for (int j = 0; j < col; j++) {
		cout << "══";
	}
	cout << "═╗";
	// 右侧对称填充
	cct_showch(start_x + col * 2 + 1, start_y + 1, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
	cct_showch(start_x + col * 2 + 1, start_y + 2 + row, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
	// 显示数据行，每行Y坐标相对递增
	for (int i = 0; i < row; i++) {
		// 显示左边框
		cct_gotoxy(start_x - 2, start_y + 2 + i);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "║";
		// 先填充整行白底区域（从边框内部开始到边框内部结束，确保左右对称）
		for (int j = 0; j < col * 2 + 3; j++) {
			cct_showch(start_x - 1 + j, start_y + 2 + i, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
		}
		// 显示数据内容（在白底上显示球）
		for (int j = 0; j < col; j++) {
			if (arr[i][j] == 'O') {
				cct_gotoxy(start_x + j * 2, start_y + 2 + i);
				cct_setcolor(COLOR_HBLUE, COLOR_BLACK);
				cout << "〇";
			}
		}
		// 显示右边框
		cct_gotoxy(start_x + col * 2, start_y + 2 + i);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "║";
	}
	// 显示底部框线，相对于最后一行
	cct_gotoxy(start_x - 2, start_y + 2 + row);
	cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
	cout << "╚";
	for (int j = 0; j < col; j++) {
		cout << "══";
	}
	cout << "═╝";
}

void printDEFG_without_devider(int row, int col, char arr[15][15], char sel)
{
	cct_cls();
	cct_setfontsize("新宋体", 36);
	if (sel == 'D') {
		cct_setconsoleborder(2 * col + 7, 100);
		draw_data_area(modeD_a_X, modeD_a_Y, row, col, arr, 'D');
	}
	else if (sel == 'E' || sel == 'F') {
		cct_setconsoleborder(6 * col + 15, 110);
		// 模式E：先绘制列提示和行提示区域，然后复用D模式数据区域逻辑
		int max_row_group = 0, max_col_group = 0;
		int row_groups[15][15] = { 0 }, row_group_count[15] = { 0 };
		int col_groups[15][15] = { 0 }, col_group_count[15] = { 0 };
		// 复用B模式的提示数字计算逻辑
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
		// 计算数据区域起始坐标（作为基准位置）
		int modeE_a_x = max_row_group * 2 + 8;
		int modeE_a_y = max_col_group + 3;
		// 1. 绘制列提示区域，基于draw_data_area的横坐标边界计算
		// draw_data_area的横坐标边界：左边界(modeE_a_x - 2)，右边界(modeE_a_x + col * 2 + 1)
		// 列提示区域顶部边框，从y=1开始
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cct_gotoxy(modeE_a_x + 2 * col + 1, 1);
		cout << " ";
		cct_gotoxy(modeE_a_x - 2, 1);
		cout << "╔";
		for (int j = 0; j < col; j++) {
			cout << "══";
		}
		cout << "═╗";

		// 列提示内容
		for (int g = 0; g < max_col_group; g++) {
			// 左边框，与draw_data_area左边框位置一致
			cct_gotoxy(modeE_a_x - 2, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 填充白底，与draw_data_area内部白底区域一致
			for (int j = 0; j < col * 2 + 3; j++) {
				cct_showch(modeE_a_x - 1 + j, 2 + g, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
			}

			// 显示列提示数字（白底黑字）
			cct_gotoxy(modeE_a_x, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			for (int j = 0; j < col; j++) {
				int blank = max_col_group - col_group_count[j];
				if (g < blank)
					cout << "  ";
				else
					cout << col_groups[j][g - blank] << " ";
			}

			// 右边框，与draw_data_area右边框位置一致
			cct_gotoxy(modeE_a_x + col * 2, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";
		}

		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cct_gotoxy(modeE_a_x + col * 2, modeE_a_y);
		cout << "║ ";
		cct_gotoxy(modeE_a_x + col * 2, modeE_a_y + 1);
		cout << "╣ ";
		cct_gotoxy(modeE_a_x - 2, modeE_a_y);
		cout << "║ ";
		cct_gotoxy(modeE_a_x - 2, modeE_a_y - 1);
		cout << "╠";
		for (int i = 0; i < col; i++) {
			cout << "══";
		}
		cout << "═╣ ";


		// 2. 绘制左侧区域（行提示和行标识），基于draw_data_area的位置计算
		// 左侧区域顶部边框，与数据区域顶部边框在同一水平线
		// draw_data_area的顶部边框位置：(modeE_a_x - 2, modeE_a_y + 1)
		cct_gotoxy(0, modeE_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╦";
		for (int j = 0; j < 4; j++)
			cout << "═";
		cout << "╦";

		// 显示行提示和行标识
		for (int i = 0; i < row; i++) {
			// 行提示区域左边框，与数据区域数据行在同一水平线
			// draw_data_area的数据行位置：(modeE_a_x - 2, modeE_a_y + 2 + i)
			cct_gotoxy(0, modeE_a_y + 2 + i);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 填充行提示区域白底
			for (int j = 0; j < max_row_group * 2 + 1; j++) {
				cct_showch(1 + j, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
			}

			// 显示行提示数字（右对齐，白底黑字）
			cct_gotoxy(1, modeE_a_y + 2 + i);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			for (int g = 0; g < max_row_group - row_group_count[i]; g++)
				cout << "  ";
			for (int g = 0; g < row_group_count[i]; g++)
				cout << setiosflags(ios::right) << setw(2) << row_groups[i][g];

			// 行提示和行标识间的分割
			cct_gotoxy(max_row_group * 2 + 2, modeE_a_y + 2 + i);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 填充行标识区域白底（4格宽）
			for (int j = 0; j < 4; j++) {
				cct_showch(max_row_group * 2 + 3 + j, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
			}

			// 显示行标识（白底黑字，居中显示）
			cct_gotoxy(max_row_group * 2 + 5, modeE_a_y + 2 + i);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << (char)('A' + i);

			// 行标识和数据区间的分割
			cct_gotoxy(max_row_group * 2 + 7, modeE_a_y + 2 + i);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";
		}

		// 3. 调用通用数据区域绘制函数（draw_data_area会绘制完整的数据区域边框）
		draw_data_area(modeE_a_x, modeE_a_y, row, col, arr, 'E');
		cct_gotoxy(modeE_a_x + col * 2, modeE_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╣ ";//注意：这句不能删，不然会乱码

		cct_gotoxy(modeE_a_x - 2, modeE_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╣ ";
		cct_gotoxy(modeE_a_x - 2, modeE_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╠ ";
		// 4. 绘制左侧区域的底部边框（与draw_data_area的底部边框连接）
		// draw_data_area的底部边框位置是：(modeE_a_x - 2, modeE_a_y + 2 + row)
		cct_gotoxy(0, modeE_a_y + 2 + row);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╩";
		for (int j = 0; j < 3; j++)
			cout << "═";
		cout << "╩";
		// 连接到数据区域底部边框的起始位置
		for (int j = 0; j < (modeE_a_x - 2) - (max_row_group * 2 + 8); j++)
			cout << "═";

		if (sel == 'F') {
			cct_gotoxy(0, 0);  // 放在最顶部
			cct_setcolor(COLOR_BLACK, COLOR_WHITE);
			cout << "测试键盘/鼠标左键/右键，按回车退出";
		}
	}
	else if (sel == 'I') {
		// I模式：使用H模式的8字符宽度格子，但添加提示数字
		// 计算提示数字
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

		cct_setfontsize("新宋体", 24);
		cct_setconsoleborder(col * 8 + max_row_group * 2 + 15, row * 4 + max_col_group + 10);

		// 计算坐标
		int modeI_a_x = max_row_group * 2 + 9;
		int modeI_a_y = max_col_group + 3;

		// 1. 绘制列提示区域
		cct_gotoxy(modeI_a_x - 4, 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═
			if (j < col - 1)
				cout << "╦";
		}
		cout << "╗ ";

		// 列提示内容
		for (int g = 0; g < max_col_group; g++) {
			cct_gotoxy(modeI_a_x - 4, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";
			for (int j = 0; j < col; j++) {
				int blank = max_col_group - col_group_count[j];
				if (g < blank) {
					cout << "       ";  // 7个空格
				}
				else {
					// 居中显示提示数字
					int num = col_groups[j][g - blank];
					cct_gotoxy(modeI_a_x + j * 8, 2 + g);
					cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
					cout << num;
				}
				if (j < col - 1) {
					cout << "║";
				}
			}
			cout << "║ ";
		}

		// 列提示和列标识间的分隔线
		cct_gotoxy(modeI_a_x - 4, modeI_a_y - 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╠";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═
			if (j < col - 1)
				cout << "╬";
		}
		cout << "╣ ";

		// 2. 绘制列标签 (a, b, c, d, e)
		cct_setcolor(COLOR_BLACK, COLOR_WHITE);
		for (int j = 0; j < col; j++) {
			cct_gotoxy(modeI_a_x + j * 8, modeI_a_y);
			cout << (char)('a' + j);
		}

		// 3. 绘制左侧行提示区域顶部边框
		cct_gotoxy(0, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╦";
		for (int j = 0; j < 4; j++)
			cout << "═";
		cout << "╦";

		// 4. 调用H模式的主体逻辑，但调整坐标
		// 绘制顶部框线：╔══════╦══════╦══════╦══════╦══════╗
		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╦";
		}
		cout << "╗ ";

		for (int i = 0; i < row; i++) {
			// 绘制行提示数字
			cct_gotoxy(1, modeI_a_y + 2 + i * 4);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			for (int g = 0; g < max_row_group - row_group_count[i]; g++)
				cout << "  ";
			for (int g = 0; g < row_group_count[i]; g++)
				cout << setiosflags(ios::right) << setw(2) << row_groups[i][g];

			// 行提示和行标识间的分割
			cct_gotoxy(max_row_group * 2 + 2, modeI_a_y + 2 + i * 4);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 显示行标识A、B、C等
			cct_gotoxy(max_row_group * 2 + 5, modeI_a_y + 2 + i * 4);
			cct_setcolor(COLOR_BLACK, COLOR_WHITE);
			cout << (char)('A' + i);

			// 行标识和数据区间的分割
			cct_gotoxy(max_row_group * 2 + 7, modeI_a_y + 2 + i * 4);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 复用H模式的3行逻辑
			for (int j = 0; j < 3; j++) {
				// 左侧边框
				cct_gotoxy(0, modeI_a_y + 2 + j + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "║";
				for (int k = 0; k < max_row_group * 2 + 1; k++)
					cout << " ";
				cout << "║";
				for (int k = 0; k < 4; k++)
					cout << " ";
				cout << "║";

				// 数据区域
				cct_gotoxy(modeI_a_x - 4, modeI_a_y + 2 + j + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "║";
				for (int k = 0; k < col; k++) {
					cout << "       ";  // 每个格子7个空格
					if (k < col - 1)
						cout << "║";
				}
				cout << "║ ";
			}

			// 第二段代码（分隔线）- 除了最后一行
			if (i < row - 1) {
				// 左侧分隔线
				cct_gotoxy(0, modeI_a_y + 5 + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "╠";
				for (int j = 0; j < max_row_group * 2 + 1; j++)
					cout << "═";
				cout << "╬";
				for (int j = 0; j < 4; j++)
					cout << "═";
				cout << "╬";

				// 数据区域分隔线
				cct_gotoxy(modeI_a_x - 4, modeI_a_y + 5 + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "╠";
				for (int j = 0; j < col; j++) {
					cout << "═══════";  // 每个格子7个空格
					if (j < col - 1)
						cout << "╬";
				}
				cout << "╣ ";
			}
		}

		// 绘制底部框线
		// 左侧底边框
		cct_gotoxy(0, modeI_a_y + 1 + (row - 1) * 4 + 4);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╩";
		for (int j = 0; j < 4; j++)
			cout << "═";
		cout << "╩";

		// 数据区域底边框
		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1 + (row - 1) * 4 + 4);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╩";
		}
		cout << "╝ ";

		// 绘制球
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				if (arr[i][j] == 'O')
					print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
			}
		}
	}

	// 恢复默认颜色
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
}

void modeD(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printDEFG_without_devider(row, col, arr, 'D');
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

void modeE(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printDEFG_without_devider(row, col, arr, 'E');
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

void modeF(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printDEFG_without_devider(row, col, arr, 'F');
	cct_enable_mouse();

	// 计算数据区域的坐标范围（与modeE中的计算一致）
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
	int modeE_a_x = max_row_group * 2 + 8;
	int modeE_a_y = max_col_group + 3;

	// 数据区域的实际范围：
	// X: [modeE_a_x, modeE_a_x + col * 2 - 1]，每2个字符代表一列，偶数位置是球位置
	// Y: [modeE_a_y + 2, modeE_a_y + 2 + row - 1]，每一行对应一个数据行

	// 键鼠循环
	int X = 0, Y = 0;
	int ret, maction;
	int keycode1, keycode2;
	bool loop = 1;
	int MX_old = -1, MY_old = -1; // 防止鼠标在同一位置时重复更新显示
	cct_setcursor(CURSOR_INVISIBLE);//关光标，晃眼睛
	while (loop) {
		/* 读键盘/鼠标，返回值为下述的某一个, 当前鼠标位置在<X,Y>处 */
		ret = cct_read_keyboard_and_mouse(X, Y, maction, keycode1, keycode2);

		if (ret == CCT_MOUSE_EVENT) {
			if (X != MX_old || Y != MY_old) {
				cct_gotoxy(11, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "[当前光标] ";

				// 判断是否在数据区域内
				bool valid = false;
				int hanghao = -1, liehao = -1;
				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeE_a_y + 2 && Y < modeE_a_y + 2 + row) {
					hanghao = Y - (modeE_a_y + 2);
					// 检查X坐标是否在数据区域范围内
					if (X >= modeE_a_x && X < modeE_a_x + col * 2) {
						// 计算列位置，每2个字符代表一列
						liehao = (X - modeE_a_x) / 2;
						// 检查是否在球的位置上（偶数位置且在有效列范围内）
						if ((X - modeE_a_x) % 2 == 0 && liehao < col) {
							valid = true;
						}
					}
				}
				if (valid)
					cout << (char)('A' + hanghao) << "行" << (char)('a' + liehao) << "列  ";
				else
					cout << "位置非法";

				MX_old = X;
				MY_old = Y;
			}

			if (maction == MOUSE_LEFT_BUTTON_CLICK) {
				cct_gotoxy(12, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到左键";
			}
			if (maction == MOUSE_RIGHT_BUTTON_CLICK) {
				cct_gotoxy(12, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到右键";
			}
		}
		else if (ret == CCT_KEYBOARD_EVENT) {
			/* 显示读到的键码 */
			show_keycode_info(11, modeE_a_y + row + 3, keycode1, keycode2);

			// 回车键退出
			if (keycode1 == 13) {
				cct_gotoxy(11, modeE_a_y + row + 3);
				cout << "[读到回车键]          ";
				loop = 0;
			}
		}
	}
	cct_setcursor(CCT_CURSOR_VISIBLE_NORMAL);
	cct_disable_mouse();
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

void modeG(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	char user_arr[15][15];
	// 生成答案数组（隐藏，仅用于验证和生成提示数字）
	inner_array(row, col, arr);
	// 初始化用户答案数组为空白
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			user_arr[i][j] = 0;
		}
	}
	bool cheat_mode = false;
	// 绘制初始界面：显示提示数字但数据区为空白（类似模式E但数据区为空）
	printDEFG_without_devider(row, col, arr, 'E');
	// 清空数据区域，只保留提示数字
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

	int modeE_a_x = max_row_group * 2 + 8;
	int modeE_a_y = max_col_group + 3;

	// 清空数据区域（只保留提示数字）
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			cct_showch(modeE_a_x + j * 2, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
			cct_showch(modeE_a_x + j * 2 + 1, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
		}
	}

	// 在顶部显示操作提示
	cct_gotoxy(0, 0);
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
	cout << "左键选〇/右键选×,Y/y提交,Z/z作弊,Q/q结束";

	// 启用鼠标
	cct_enable_mouse();

	bool loop = 1;
	int X = 0, Y = 0;
	int ret, maction;
	int keycode1, keycode2;
	int MX_old = -1, MY_old = -1; // 防止鼠标在同一位置时重复更新显示
	bool valid = false;
	int hanghao = -1, liehao = -1;
	cct_setcursor(CURSOR_INVISIBLE);//关光标，晃眼睛

	while (loop) {
		// 读取键盘和鼠标输入
		ret = cct_read_keyboard_and_mouse(X, Y, maction, keycode1, keycode2);

		// 处理鼠标事件
		if (ret == CCT_MOUSE_EVENT) {
			// 处理鼠标移动事件：显示当前光标位置
			if (X != MX_old || Y != MY_old) {
				// 清除并重新显示光标位置信息
				cct_gotoxy(11, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "[当前光标] ";

				// 判断鼠标是否在数据区域内的有效位置
				bool valid = false;
				int hanghao = -1, liehao = -1;

				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeE_a_y + 2 && Y < modeE_a_y + 2 + row) {
					hanghao = Y - (modeE_a_y + 2);
					// 检查X坐标是否在数据区域范围内
					if (X >= modeE_a_x && X < modeE_a_x + col * 2) {
						// 计算列位置，每2个字符代表一列
						liehao = (X - modeE_a_x) / 2;
						// 检查是否在球的位置上（偶数位置且在有效列范围内）
						if ((X - modeE_a_x) % 2 == 0 && liehao < col) {
							valid = true;
						}
					}
				}

				// 显示位置信息
				if (valid)
					cout << (char)('A' + hanghao) << "行" << (char)('a' + liehao) << "列  ";
				else
					cout << "位置非法                ";  // 添加足够的空格清除之前的内容

				// 更新鼠标位置记录
				MX_old = X;
				MY_old = Y;
			}

			// 处理鼠标左键点击事件：选择/取消选择球
			if (maction == MOUSE_LEFT_BUTTON_CLICK) {
				// 检查是否在有效数据区域内
				bool click_valid = false;
				int click_hanghao = -1, click_liehao = -1;
				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeE_a_y + 2 && Y < modeE_a_y + 2 + row) {
					click_hanghao = Y - (modeE_a_y + 2);
					// 检查X坐标是否在数据区域范围内
					if (X >= modeE_a_x && X < modeE_a_x + col * 2) {
						// 计算列位置，每2个字符代表一列
						click_liehao = (X - modeE_a_x) / 2;
						// 检查是否在球的位置上（偶数位置且在有效列范围内）
						if ((X - modeE_a_x) % 2 == 0 && click_liehao < col) {
							click_valid = true;
						}
					}
				}

				if (click_valid) {
					// 切换状态：如果已经是'O'，则清除；否则设置为'O'
					if (user_arr[click_hanghao][click_liehao] == 'O')
						user_arr[click_hanghao][click_liehao] = 0;
					else
						user_arr[click_hanghao][click_liehao] = 'O';

					// 重新绘制该位置
					cct_gotoxy(modeE_a_x + click_liehao * 2, modeE_a_y + 2 + click_hanghao);
					if (user_arr[click_hanghao][click_liehao] == 'O') {
						if (cheat_mode) {
							if (arr[click_hanghao][click_liehao] == 'O')
								cct_setcolor(COLOR_HBLUE, COLOR_HWHITE);
							else
								cct_setcolor(COLOR_HRED, COLOR_HWHITE);
						}
						else
							cct_setcolor(COLOR_HBLUE, COLOR_HWHITE);
						cout << "〇";
					}
					else {
						// 空白位置，默认白底
						if (cheat_mode && arr[click_hanghao][click_liehao] == 'O') {
							cct_setcolor(COLOR_WHITE, COLOR_BLACK);
							cout << "〇";
						}
						else {
							// 清空位置，保持白底
							cct_showch(modeE_a_x + click_liehao * 2, modeE_a_y + 2 + click_hanghao, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
							cct_showch(modeE_a_x + click_liehao * 2 + 1, modeE_a_y + 2 + click_hanghao, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
						}
					}
					cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				}

				// 在底部显示操作反馈
				cct_gotoxy(12, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_HWHITE);
				cout << "读到左键";
			}

			// 处理鼠标右键点击事件：设置/取消设置叉号
			if (maction == MOUSE_RIGHT_BUTTON_CLICK) {
				// 检查是否在有效数据区域内
				bool click_valid = false;
				int click_hanghao = -1, click_liehao = -1;
				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeE_a_y + 2 && Y < modeE_a_y + 2 + row) {
					click_hanghao = Y - (modeE_a_y + 2);
					// 检查X坐标是否在数据区域范围内
					if (X >= modeE_a_x && X < modeE_a_x + col * 2) {
						// 计算列位置，每2个字符代表一列
						click_liehao = (X - modeE_a_x) / 2;
						// 检查是否在球的位置上（偶数位置且在有效列范围内）
						if ((X - modeE_a_x) % 2 == 0 && click_liehao < col) {
							click_valid = true;
						}
					}
				}

				if (click_valid) {
					// 右键设置为'X'（打叉）或清除
					if (user_arr[click_hanghao][click_liehao] == 'X')
						user_arr[click_hanghao][click_liehao] = 0;
					else
						user_arr[click_hanghao][click_liehao] = 'X';


					// 重新绘制该位置
					cct_gotoxy(modeE_a_x + click_liehao * 2, modeE_a_y + 2 + click_hanghao);
					if (user_arr[click_hanghao][click_liehao] == 'X') {
						if (cheat_mode) {
							if (arr[click_hanghao][click_liehao] == 'O') {
								// 作弊模式：该位置有球（提示用户这里不应该放叉）
								cct_setcolor(COLOR_HBLUE, COLOR_BLACK);
							}
							else {
								// 作弊模式：该位置无球（正确的叉号位置）
								cct_setcolor(COLOR_HRED, COLOR_BLACK);
							}
						}
						else {
							cct_setcolor(COLOR_HRED, COLOR_BLACK);
						}
						cout << "×";
					}
					else {
						if (cheat_mode && arr[click_hanghao][click_liehao] == 'O') {
							cct_setcolor(COLOR_WHITE, COLOR_BLACK); // 作弊提示：白底黑字
							cout << "〇"; // 提示应该有球
						}
						else {
							// 清空位置，保持白底
							cct_showch(modeE_a_x + click_liehao * 2, modeE_a_y + 2 + click_hanghao, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
							cct_showch(modeE_a_x + click_liehao * 2 + 1, modeE_a_y + 2 + click_hanghao, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
						}
					}
					cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				}

				// 在底部显示操作反馈
				cct_gotoxy(12, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_HWHITE);
				cout << "读到右键";
			}
		} // 结束鼠标事件处理	
			// 处理键盘事件
		else if (ret == CCT_KEYBOARD_EVENT) {
			// Y/y键：提交答案并检查正确性
			if (keycode1 == 'Y' || keycode1 == 'y') {
				if (check_answer(arr, user_arr, row, col)) {
					// 答案正确，游戏结束
					cct_gotoxy(0, modeE_a_y + row + 5);
					cct_setcolor(COLOR_BLACK, COLOR_HWHITE);
					cout << "提交成功，游戏结束";
					to_be_continued();
					cct_setfontsize("新宋体", 16);
					cct_setconsoleborder(120, 30);
					return;
				}
				else {
					// 答案错误，提示可以使用作弊模式查看
					cct_gotoxy(0, modeE_a_y + row + 4);
					cct_setcolor(COLOR_BLACK, COLOR_HWHITE);
					cout << "提交错误, 可用作弊模式查看";
					Sleep(1000);
					cct_gotoxy(0, modeE_a_y + row + 4);
					cout << "                              ";
				}
			}
			// Z/z键：切换作弊模式
			else if (keycode1 == 'Z' || keycode1 == 'z') {
				cheat_mode = !cheat_mode;
				cct_gotoxy(11, modeE_a_y + row + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				if (cheat_mode)
					cout << "[作弊模式开]      ";
				else
					cout << "[作弊模式关]      ";

				// 重新绘制整个数据区域以应用作弊模式的颜色变化
				for (int i = 0; i < row; i++) {
					for (int j = 0; j < col; j++) {
						cct_gotoxy(modeE_a_x + j * 2, modeE_a_y + 2 + i);

						// 处理用户已放置的球
						if (user_arr[i][j] == 'O') {
							if (cheat_mode) {
								// 作弊模式：根据正确性显示不同颜色
								if (arr[i][j] == 'O')
									cct_setcolor(COLOR_HBLUE, COLOR_BLACK);
								else
									cct_setcolor(COLOR_HRED, COLOR_BLACK);
							}
							else
								cct_setcolor(COLOR_HBLUE, COLOR_BLACK);
							cout << "〇";
							cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
						}
						// 处理用户已放置的叉号
						else if (user_arr[i][j] == 'X') {
							if (cheat_mode) {
								if (arr[i][j] == 'O') {
									// 作弊模式：该位置有球 → 蓝字黑底（提示用户这里不应该放叉）
									cct_setcolor(COLOR_HBLUE, COLOR_BLACK);
								}
								else {
									// 作弊模式：该位置无球 → 红字黑底（正确的叉号位置）
									cct_setcolor(COLOR_HRED, COLOR_BLACK);
								}
							}
							else {
								cct_setcolor(COLOR_HRED, COLOR_BLACK);
							}
							cout << "×";
							cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
						}
						// 处理空白位置
						else {
							if (cheat_mode && arr[i][j] == 'O') {
								cct_setcolor(COLOR_WHITE, COLOR_BLACK);
								cout << "〇"; // 提示应该有球
							}
							else {
								// 清空位置，使用cct_showch确保正确的白底
								cct_showch(modeE_a_x + j * 2, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
								cct_showch(modeE_a_x + j * 2 + 1, modeE_a_y + 2 + i, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
							}
						}
					}
				}
			}
			// Q/q键：退出游戏
			else if (keycode1 == 'Q' || keycode1 == 'q') {
				cct_gotoxy(0, modeE_a_y + row + 3);
				cout << "[读到Q/q，游戏结束]            ";
				loop = 0; // 设置循环标志为0，退出游戏循环
			}
			else {
				// 其他键：显示键码提示
				show_keycode_info(11, modeE_a_y + row + 3, keycode1, keycode2);
			}
		} // end of CCT_KEYBOARD_EVENT
	}

	cct_setcursor(CCT_CURSOR_VISIBLE_NORMAL);
	cct_disable_mouse();
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

// print_HIJKballs函数：绘制球形状，用于模式H-K
void print_HIJKballs(int x, int y, int fg_color, int bg_color, char ball)
{
	// 第1行球形状：╔═══╗ （在第2行，即y+1）
	cct_gotoxy(x + 1, y + 1);
	cct_setcolor(fg_color, bg_color);
	cout << "╔═══╗ ";
	// 第2行球形状：║ symbol ║ （在第3行，即y+2）
	cct_gotoxy(x + 1, y + 2);
	cct_setcolor(fg_color, bg_color);
	if (ball == 'O')
		cout << "║ 〇║ ";//看着不对称，〇占两字节所以完美对称
	else if (ball == 'X')
		cout << "║ ×║ ";
	// 第3行球形状：╚═══╝ （在第4行，即y+3）
	cct_gotoxy(x + 1, y + 3);
	cct_setcolor(fg_color, bg_color);
	cout << "╚═══╝ ";
}

// printIJKL_with_devider函数：绘制IJKL模式的公共函数（有分隔线版本）
void printIJKL_with_devider(int row, int col, char arr[15][15], char sel)
{
	if (sel == 'H') {
		// H模式：纯数据显示，无提示数字
		cct_cls();
		// 根据区域大小动态调整字体
		int fontsize;
		if (row <= 5)
			fontsize = 24;
		else if (row <= 10)
			fontsize = 16;
		else
			fontsize = 12;
		cct_setfontsize("新宋体", fontsize);
		cct_setconsoleborder(col * 8 + 5, row * 4 + 5);

		// 绘制列标签 (a, b, c, d, e) - 每个格子8字符间隔，居中
		cct_setcolor(COLOR_BLACK, COLOR_WHITE);
		for (int j = 0; j < col; j++) {
			cct_gotoxy(modeH_a_X + j * 8, modeH_a_Y);
			cout << (char)('a' + j);
		}

		// 绘制行标签 (A, B, C, D, E) - 每行4字符间隔，居中显示
		for (int i = 0; i < row; i++) {
			cct_gotoxy(0, modeH_a_Y + 3 + i * 4);
			cout << (char)('A' + i);
		}

		// 绘制顶部框线：╔══════╦══════╦══════╦══════╦══════╗
		cct_gotoxy(modeH_a_X - 4, modeH_a_Y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╦";
		}
		cout << "╗ ";

		for (int i = 0; i < row; i++) {
			for (int j = 0; j < 3; j++) {
				cct_gotoxy(modeH_a_X - 4, modeH_a_Y + 2 + j + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "║";
				for (int k = 0; k < col; k++) {
					cout << "       ";  // 每个格子7个空格
					if (k < col - 1)
						cout << "║";
				}
				cout << "║ ";
			}

			// 第二段代码（分隔线）- 除了最后一行
			if (i < row - 1) {
				cct_gotoxy(modeH_a_X - 4, modeH_a_Y + 5 + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "╠";
				for (int j = 0; j < col; j++) {
					cout << "═══════";  // 每个格子7个空格
					if (j < col - 1)
						cout << "╬";
				}
				cout << "╣ ";
			}
		}

		// 绘制底部框线
		cct_gotoxy(modeH_a_X - 4, modeH_a_Y + 1 + row * 4);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╩";
		}
		cout << "╝ ";

		// 绘制球
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				if (arr[i][j] == 'O')
					print_HIJKballs(modeH_a_X - 3 + j * 8, modeH_a_Y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
			}
		}
	}
	else if (sel == 'I' || sel == 'J') {
		// I/J模式：添加提示数字
		// 计算提示数字
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

		cct_cls();
		// 根据区域大小动态调整字体
		int fontsize;
		if (row <= 5)
			fontsize = 24;
		else if (row <= 10)
			fontsize = 16;
		else
			fontsize = 12;
		cct_setfontsize("新宋体", fontsize);
		cct_setconsoleborder(col * 8 + max_row_group * 2 + 15, row * 4 + max_col_group + 10);

		// 计算坐标
		int modeI_a_x = max_row_group * 2 + 10;
		int modeI_a_y = max_col_group + 3;

		// 1. 绘制列提示区域
		cct_gotoxy(modeI_a_x - 4, 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < 8 * col - 1; j++) {
			cout << "═";
		}
		cout << "╗ ";

		// 列提示内容
		for (int g = 0; g < max_col_group; g++) {
			// 先填充整行白底
			for (int k = 0; k < col * 8 + 2; k++) {
				cct_showch(modeI_a_x - 4 + k, 2 + g, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
			}

			// 绘制左边框
			cct_gotoxy(modeI_a_x - 4, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║";

			// 显示列提示数字
			for (int j = 0; j < col; j++) {
				int blank = max_col_group - col_group_count[j];
				if (g >= blank) {
					// 居中显示提示数字
					int num = col_groups[j][g - blank];
					cct_gotoxy(modeI_a_x + j * 8, 2 + g);
					cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
					cout << num;
				}

			}

			// 绘制右边框
			cct_gotoxy(modeI_a_x + 4 + (col - 1) * 8, 2 + g);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << "║ ";
		}

		// 列提示和列标识间的分隔线
		cct_gotoxy(modeI_a_x - 4, modeI_a_y - 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╠";
		for (int j = 0; j < 8 * col - 1; j++) {
			cout << "═";
		}
		cout << "╣ ";

		// 2. 绘制列标签 (a, b, c, d, e) 
		// 先填充整行白底 - 从左边框到右边框
		for (int k = 0; k < col * 8 + 2; k++) {
			cct_showch(modeI_a_x - 4 + k, modeI_a_y, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
		}
		for (int j = 0; j < col; j++) {
			cct_gotoxy(modeI_a_x + j * 8, modeI_a_y);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << (char)('a' + j);
		}
		cct_gotoxy(modeI_a_x - 4, modeI_a_y);
		cout << "║";
		cct_gotoxy(modeI_a_x + col * 8 - 4, modeI_a_y);
		cout << "║";

		// 3. 绘制左侧行提示区域顶部边框 - ABCDE区域宽度为4
		cct_gotoxy(0, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╦";
		for (int j = 0; j < 4; j++)  // ABCDE区域宽度为4
			cout << "═";
		cout << "╬";

		// 4. 绘制数据区域顶部框线
		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╔";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╦";
		}
		cout << "╗ ";

		for (int i = 0; i < row; i++) {
			// 复用H模式的第2-4行逻辑
			for (int j = 0; j < 4; j++) {
				// 先填充行提示数字区域白底
				for (int k = 0; k < max_row_group * 2 + 1; k++) {
					cct_showch(1 + k, modeI_a_y + 2 + j + i * 4, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
				}

				// 填充ABCDE区域白底
				for (int k = 0; k < 4; k++) {
					cct_showch(max_row_group * 2 + 3 + k, modeI_a_y + 2 + j + i * 4, ' ', COLOR_HWHITE, COLOR_BLACK, 1);
				}

				// 左侧边框
				cct_gotoxy(0, modeI_a_y + 2 + j + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "║";
				cct_gotoxy(max_row_group * 2 + 2, modeI_a_y + 2 + j + i * 4);
				cout << "║";
				cct_gotoxy(max_row_group * 2 + 7, modeI_a_y + 2 + j + i * 4);
				cout << "║";

				// 数据区域
				cct_gotoxy(modeI_a_x - 4, modeI_a_y + 2 + j + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "║";
				for (int k = 0; k < col; k++) {
					cout << "       ";  // 每个格子7个空格
					if (k < col - 1)
						cout << "║";
				}
				cout << "║ ";
			}

			// 第二段代码（分隔线）- 除了最后一行
			if (i < row - 1) {
				// 数据区域分隔线
				cct_gotoxy(modeI_a_x - 4, modeI_a_y + 5 + i * 4);
				cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
				cout << "╠";
				for (int j = 0; j < col; j++) {
					cout << "═══════";  // 每个格子7个空格
					if (j < col - 1)
						cout << "╬";
				}
				cout << "╣ ";
			}
			// 第一行：绘制行提示数字、行标识和球
			// 绘制行提示数字
			cct_gotoxy(1, modeI_a_y + 3 + i * 4);
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			for (int g = 0; g < max_row_group - row_group_count[i]; g++)
				cout << "  ";
			for (int g = 0; g < row_group_count[i]; g++)
				cout << setiosflags(ios::right) << setw(2) << row_groups[i][g];

			// 显示行标识A、B、C等
			cct_gotoxy(max_row_group * 2 + 5, modeI_a_y + 3 + i * 4);  // 居中显示
			cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
			cout << (char)('A' + i);
		}

		// 绘制底部框线
		// 左侧底边框
		cct_gotoxy(0, modeI_a_y + 1 + row * 4);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < max_row_group * 2 + 1; j++)
			cout << "═";
		cout << "╩";
		for (int j = 0; j < 4; j++)  // ABCDE区域宽度为4
			cout << "═";
		cout << "╩";

		// 数据区域底边框
		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1 + row * 4);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╚";
		for (int j = 0; j < col; j++) {
			cout << "═══════";  // 每个格子7个═，解决╦占位的问题
			if (j < col - 1)
				cout << "╩";
		}
		cout << "╝ ";

		cct_gotoxy(modeI_a_x + col * 8 - 4, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╣";
		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╠";
		for (int i = 0; i < col; i++) {
			cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1);
			cout << " ";
		}

		cct_gotoxy(modeI_a_x - 4, modeI_a_y + 1);
		cct_setcolor(COLOR_HWHITE, COLOR_BLACK);
		cout << "╬";

		// 绘制球
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				if (arr[i][j] == 'O')
					print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
			}
		}

		if (sel == 'J') {
			cct_gotoxy(0, 0);  // 放在最顶部
			cct_setcolor(COLOR_BLACK, COLOR_WHITE);
			cout << "测试键盘/鼠标左键/右键，按回车退出";
		}
	}

	// 恢复默认颜色
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
}

// modeH函数：绘制伪图形界面下画出初始状态（有分隔线版本）
void modeH(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printIJKL_with_devider(row, col, arr, 'H');
	cout << endl << endl << endl;
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

// modeI函数：在modeH基础上添加行列提示数字
void modeI(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printIJKL_with_devider(row, col, arr, 'I');
	cout << endl << endl << endl;
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

// modeJ函数：在modeI基础上添加鼠标交互
void modeJ(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	inner_array(row, col, arr);
	printIJKL_with_devider(row, col, arr, 'J');
	cct_enable_mouse();

	// 计算数据区域的坐标范围（与modeI中的计算一致）
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
	int modeI_a_x = max_row_group * 2 + 10;
	int modeI_a_y = max_col_group + 3;

	// 数据区域的实际范围（J模式使用8字符宽度格子）：
	// X: [modeI_a_x - 3, modeI_a_x + col * 8 - 5]，每8个字符代表一列，球在中心3×3区域
	// Y: [modeI_a_y + 2, modeI_a_y + 2 + row * 4 - 1]，每4行对应一个数据行，球在第2-4行

	// 键鼠循环
	int X = 0, Y = 0;
	int ret, maction;
	int keycode1, keycode2;
	bool loop = 1;
	int MX_old = -1, MY_old = -1; // 防止鼠标在同一位置时重复更新显示
	cct_setcursor(CURSOR_INVISIBLE);//关光标，晃眼睛
	while (loop) {
		/* 读键盘/鼠标，返回值为下述的某一个, 当前鼠标位置在<X,Y>处 */
		ret = cct_read_keyboard_and_mouse(X, Y, maction, keycode1, keycode2);

		if (ret == CCT_MOUSE_EVENT) {
			if (X != MX_old || Y != MY_old) {
				cct_gotoxy(11, modeI_a_y + row * 4 + 6);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "[当前光标] ";

				// 判断是否在数据区域内的球区域
				bool valid = false;
				int hanghao = -1, liehao = -1;
				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeI_a_y + 2 && Y < modeI_a_y + 2 + row * 4) {
					// 计算是在哪个球的行
					int ball_row = (Y - (modeI_a_y + 2)) / 4;
					// 检查是否在球的3行区域内（每个球占据4行中的第1-3行）
					int in_ball_y = (Y - (modeI_a_y + 2)) % 4;
					if (in_ball_y >= 0 && in_ball_y < 3 && ball_row < row) {
						hanghao = ball_row;
						// 检查X坐标是否在数据区域范围内
						if (X >= modeI_a_x - 3 && X < modeI_a_x + col * 8 - 3) {
							// 计算是在哪个球的列
							int ball_col = (X - (modeI_a_x - 3)) / 8;
							// 检查是否在球的6列区域内（每个球占据8列中的中间6列）
							int in_ball_x = (X - (modeI_a_x - 3)) % 8;
							if (in_ball_x >= 1 && in_ball_x < 7 && ball_col < col) {
								liehao = ball_col;
								valid = true;
							}
						}
					}
				}
				if (valid)
					cout << (char)('A' + hanghao) << "行" << (char)('a' + liehao) << "列  ";
				else
					cout << "位置非法";

				MX_old = X;
				MY_old = Y;
			}

			if (maction == MOUSE_LEFT_BUTTON_CLICK) {
				cct_gotoxy(12, modeI_a_y + row * 4 + 6);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到左键";
			}
			if (maction == MOUSE_RIGHT_BUTTON_CLICK) {
				cct_gotoxy(12, modeI_a_y + row * 4 + 6);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到右键";
			}
		}
		else if (ret == CCT_KEYBOARD_EVENT) {
			/* 显示读到的键码 */
			show_keycode_info(11, modeI_a_y + row * 4 + 6, keycode1, keycode2);

			// 回车键退出
			if (keycode1 == 13) {
				cct_gotoxy(11, modeI_a_y + row * 4 + 6);
				cout << "[读到回车键]          ";
				loop = 0;
			}
		}
	}
	cct_setcursor(CCT_CURSOR_VISIBLE_NORMAL);
	cct_disable_mouse();
	to_be_continued();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}

// modeK函数：在modeJ基础上添加完整游戏逻辑（类似模式G相对于模式F的增强）
void modeK(int& row, int& col)
{
	int room = get_room();
	row = room;
	col = room;
	char arr[15][15];
	char user_arr[15][15];
	// 生成答案数组（隐藏，仅用于验证和生成提示数字）
	inner_array(row, col, arr);
	// 初始化用户答案数组为空白
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			user_arr[i][j] = 0;
		}
	}
	bool cheat_mode = false;

	// 绘制初始界面：显示提示数字但数据区为空白
	printIJKL_with_devider(row, col, arr, 'I');

	// 计算坐标参数（与模式I/J一致）
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
	int modeI_a_x = max_row_group * 2 + 10;
	int modeI_a_y = max_col_group + 3;

	// 清空数据区域（只保留提示数字）- 清除球的显示
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			// 清空整个球区域（6×3的区域）
			for (int dy = 0; dy < 3; dy++) {
				for (int dx = 0; dx < 7; dx++) {
					cct_showch(modeI_a_x - 3 + j * 8 + dx, modeI_a_y + 2 + i * 4 + dy, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
				}
			}
		}
	}

	// 在顶部显示操作提示
	cct_gotoxy(0, 0);
	cct_setcolor(COLOR_BLACK, COLOR_WHITE);
	cout << "左键选〇/右键选×,Y/y提交,Z/z作弊,Q/q结束";

	// 启用鼠标
	cct_enable_mouse();

	bool loop = 1;
	int X = 0, Y = 0;
	int ret, maction;
	int keycode1, keycode2;
	int MX_old = -1, MY_old = -1;
	cct_setcursor(CURSOR_INVISIBLE);

	while (loop) {
		// 读取键盘和鼠标输入
		ret = cct_read_keyboard_and_mouse(X, Y, maction, keycode1, keycode2);

		// 处理鼠标事件
		if (ret == CCT_MOUSE_EVENT) {
			// 处理鼠标移动事件：显示当前光标位置
			if (X != MX_old || Y != MY_old) {
				cct_gotoxy(11, modeI_a_y + row * 4 + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "[当前光标] ";

				// 判断是否在数据区域内的球区域
				bool valid = false;
				int hanghao = -1, liehao = -1;

				// 检查Y坐标是否在数据区域范围内
				if (Y >= modeI_a_y + 2 && Y < modeI_a_y + 2 + row * 4) {
					// 计算是在哪个球的行
					int ball_row = (Y - (modeI_a_y + 2)) / 4;
					// 检查是否在球的3行区域内（每个球占据4行中的第1-3行）
					int in_ball_y = (Y - (modeI_a_y + 2)) % 4;
					if (in_ball_y >= 0 && in_ball_y < 3 && ball_row < row) {
						hanghao = ball_row;
						// 检查X坐标是否在数据区域范围内
						if (X >= modeI_a_x - 3 && X < modeI_a_x + col * 8 - 3) {
							// 计算是在哪个球的列
							int ball_col = (X - (modeI_a_x - 3)) / 8;
							// 检查是否在球的6列区域内（每个球占据8列中的中间6列）
							int in_ball_x = (X - (modeI_a_x - 3)) % 8;
							if (in_ball_x >= 1 && in_ball_x < 7 && ball_col < col) {
								liehao = ball_col;
								valid = true;
							}
						}
					}
				}

				// 显示位置信息
				if (valid)
					cout << (char)('A' + hanghao) << "行" << (char)('a' + liehao) << "列  ";
				else
					cout << "位置非法                ";

				MX_old = X;
				MY_old = Y;
			}

			// 处理鼠标左键点击事件：选择/取消选择球
			if (maction == MOUSE_LEFT_BUTTON_CLICK) {
				// 检查是否在有效数据区域内
				bool click_valid = false;
				int click_hanghao = -1, click_liehao = -1;

				if (Y >= modeI_a_y + 2 && Y < modeI_a_y + 2 + row * 4) {
					int ball_row = (Y - (modeI_a_y + 2)) / 4;
					int in_ball_y = (Y - (modeI_a_y + 2)) % 4;
					if (in_ball_y >= 0 && in_ball_y < 3 && ball_row < row) {
						click_hanghao = ball_row;
						if (X >= modeI_a_x - 3 && X < modeI_a_x + col * 8 - 3) {
							int ball_col = (X - (modeI_a_x - 3)) / 8;
							int in_ball_x = (X - (modeI_a_x - 3)) % 8;
							if (in_ball_x >= 1 && in_ball_x < 7 && ball_col < col) {
								click_liehao = ball_col;
								click_valid = true;
							}
						}
					}
				}

				if (click_valid) {
					// 切换状态：如果已经是'O'，则清除；否则设置为'O'
					if (user_arr[click_hanghao][click_liehao] == 'O')
						user_arr[click_hanghao][click_liehao] = 0;
					else
						user_arr[click_hanghao][click_liehao] = 'O';

					// 重新绘制该位置 - 直接处理
					// 先清空球区域
					for (int dy = 0; dy < 3; dy++) {
						for (int dx = 0; dx < 7; dx++) {
							cct_showch(modeI_a_x - 3 + click_liehao * 8 + dx, modeI_a_y + 2 + click_hanghao * 4 + dy, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
						}
					}

					if (user_arr[click_hanghao][click_liehao] == 'O') {
						// 用户放置了球
						if (cheat_mode) {
							if (arr[click_hanghao][click_liehao] == 'O') {
								// 作弊模式：该位置有球 
								print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
							}
							else {
								// 作弊模式：该位置无球
								print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HRED, COLOR_BLACK, 'O');
							}
						}
						else {
							// 非作弊模式：正常蓝色球
							print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
						}
					}
					else if (cheat_mode && arr[click_hanghao][click_liehao] == 'O') {
						// 作弊提示：应该有球但用户没选择
						print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_WHITE, COLOR_BLACK, 'O');
					}
				}

				// 在底部显示操作反馈
				cct_gotoxy(12, modeI_a_y + row * 4 + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到左键";
			}

			// 处理鼠标右键点击事件：设置/取消设置叉号
			if (maction == MOUSE_RIGHT_BUTTON_CLICK) {
				// 检查是否在有效数据区域内
				bool click_valid = false;
				int click_hanghao = -1, click_liehao = -1;

				if (Y >= modeI_a_y + 2 && Y < modeI_a_y + 2 + row * 4) {
					int ball_row = (Y - (modeI_a_y + 2)) / 4;
					int in_ball_y = (Y - (modeI_a_y + 2)) % 4;
					if (in_ball_y >= 0 && in_ball_y < 3 && ball_row < row) {
						click_hanghao = ball_row;
						if (X >= modeI_a_x - 3 && X < modeI_a_x + col * 8 - 3) {
							int ball_col = (X - (modeI_a_x - 3)) / 8;
							int in_ball_x = (X - (modeI_a_x - 3)) % 8;
							if (in_ball_x >= 1 && in_ball_x < 7 && ball_col < col) {
								click_liehao = ball_col;
								click_valid = true;
							}
						}
					}
				}

				if (click_valid) {
					// 右键设置为'X'（打叉）或清除
					if (user_arr[click_hanghao][click_liehao] == 'X')
						user_arr[click_hanghao][click_liehao] = 0;
					else
						user_arr[click_hanghao][click_liehao] = 'X';

					// 重新绘制该位置 - 直接处理
					// 先清空球区域
					for (int dy = 0; dy < 3; dy++) {
						for (int dx = 0; dx < 7; dx++) {
							cct_showch(modeI_a_x - 3 + click_liehao * 8 + dx, modeI_a_y + 2 + click_hanghao * 4 + dy, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
						}
					}
					if (user_arr[click_hanghao][click_liehao] == 'X') {
						// 用户放置了叉号
						if (cheat_mode) {
							if (arr[click_hanghao][click_liehao] == 'O') {
								// 作弊模式：该位置有球 → 蓝色框架
								print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HBLUE, COLOR_BLACK, 'X');
							}
							else {
								// 作弊模式：该位置无球 → 红色框架
								print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HRED, COLOR_BLACK, 'X');
							}
						}
						else {
							// 非作弊模式：正常红色叉号
							print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_HRED, COLOR_BLACK, 'X');
						}
					}
					else {
						// 用户没有放置叉号（空白状态）
						if (cheat_mode && arr[click_hanghao][click_liehao] == 'O') {
							// 作弊提示：这里应该有球但用户没放
							print_HIJKballs(modeI_a_x - 3 + click_liehao * 8, modeI_a_y + 1 + click_hanghao * 4, COLOR_WHITE, COLOR_BLACK, 'O');
						}
						// 否则保持空白（已经清空过了）
					}

				}

				// 在底部显示操作反馈
				cct_gotoxy(12, modeI_a_y + row * 4 + 3);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				cout << "读到右键";
			}
		}
		// 处理键盘事件
		else if (ret == CCT_KEYBOARD_EVENT) {
			// Y/y键：提交答案并检查正确性
			if (keycode1 == 'Y' || keycode1 == 'y') {
				if (check_answer(arr, user_arr, row, col)) {
					// 答案正确，游戏结束
					cct_gotoxy(0, modeI_a_y + row * 4 + 4);
					cct_setcolor(COLOR_BLACK, COLOR_WHITE);
					cout << "提交成功，游戏结束";
					to_be_continued();
					loop = 0;
				}
				else {
					// 答案错误，提示可以使用作弊模式查看
					cct_gotoxy(0, modeI_a_y + row * 4 + 4);
					cct_setcolor(COLOR_BLACK, COLOR_WHITE);
					cout << "提交错误, 可用作弊模式查看";
					Sleep(1000);
					cct_gotoxy(0, modeI_a_y + row * 4 + 4);
					cout << "                            ";
				}
			}
			// Z/z键：切换作弊模式
			else if (keycode1 == 'Z' || keycode1 == 'z') {
				cheat_mode = !cheat_mode;
				cct_gotoxy(11, modeI_a_y + row * 4 + 4);
				cct_setcolor(COLOR_BLACK, COLOR_WHITE);
				if (cheat_mode)
					cout << "[作弊模式开]       ";
				else
					cout << "[作弊模式关]       ";

				// 重新绘制整个数据区域以应用作弊模式的颜色变化
				for (int i = 0; i < row; i++) {
					for (int j = 0; j < col; j++) {
						// 先清空球区域
						for (int dy = 0; dy < 3; dy++) {
							for (int dx = 0; dx < 7; dx++) {
								cct_showch(modeI_a_x - 3 + j * 8 + dx, modeI_a_y + 2 + i * 4 + dy, ' ', COLOR_HWHITE, COLOR_HWHITE, 1);
							}
						}

						// 处理用户已放置的球
						if (user_arr[i][j] == 'O') {
							if (cheat_mode) {
								if (arr[i][j] == 'O')
									print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
								else
									print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HRED, COLOR_BLACK, 'O');
							}
							else
								print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'O');
						}
						// 处理用户已放置的叉号
						else if (user_arr[i][j] == 'X') {
							if (cheat_mode) {
								if (arr[i][j] == 'O')
									print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HBLUE, COLOR_BLACK, 'X');
								else
									print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HRED, COLOR_BLACK, 'X');
							}
							else
								print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_HRED, COLOR_BLACK, 'X');
						}
						// 处理空白位置
						else {
							if (cheat_mode && arr[i][j] == 'O') {
								print_HIJKballs(modeI_a_x - 3 + j * 8, modeI_a_y + 1 + i * 4, COLOR_WHITE, COLOR_BLACK, 'O');
							}
						}
					}
				}
			}
			// Q/q键：退出游戏
			else if (keycode1 == 'Q' || keycode1 == 'q') {
				cct_gotoxy(0, modeI_a_y + row * 4 + 4);
				cout << "[读到Q/q，游戏结束]          ";
				loop = 0;
				to_be_continued();
				return;
			}
			else {
				// 其他键：显示键码提示
				show_keycode_info(11, modeI_a_y + row * 4 + 3, keycode1, keycode2);
			}
		}
	}

	cct_setcursor(CCT_CURSOR_VISIBLE_NORMAL);
	cct_disable_mouse();
	cct_setfontsize("新宋体", 16);
	cct_setconsoleborder(120, 30);
}
