/* 2452487 ÐÅ°² ºúÖÐÆM */
#pragma once

#include "../include/cmd_console_tools.h"
#include "../include/simple_menu.h"

void to_be_continued();
int get_room();
bool check_answer(char arr[15][15], char user_arr[15][15], int row, int col);
void show_keycode_info(int x, int y, int keycode1, int keycode2);
void inner_array(int row, int col, char arr[15][15]);
void show_array(char arr[15][15], int row, int col, char sel, bool cheat_mode = false, char user_arr[15][15] = nullptr);
void modeA(int& row, int& col);
void modeB(int& row, int& col);
void modeC(int& row, int& col);
void modeD(int& row, int& col);
void modeE(int& row, int& col);
void modeF(int& row, int& col);
void modeG(int& row, int& col);
void modeH(int& row, int& col);
void modeI(int& row, int& col);
void modeJ(int& row, int& col);
void modeK(int& row, int& col);
void printDEFG_without_devider(int row, int col, char arr[15][15], char sel);
void printIJKL_with_devider(int row, int col, char arr[15][15], char sel);
void print_jiajianA(int row, int col);
void print_jiajianB(int row, int col, int max_row_group, bool sign_jiahao);
void get_groups(const char* line, int len, int* groups, int& group_count);
void draw_data_area(int start_x, int start_y, int row, int col, char arr[15][15], char sel);
void print_HIJKballs(int x, int y, int fg_color, int bg_color, char ball);
void output_something();
