/* 2452487 信安 胡中芃 */
#pragma once
#include <string>
using namespace std;

#define bytes_per_line 16 // hex一行打印16个字节

void trim_left(string& s);//去掉左侧空格/tab/
void trim_right(string& s);//去掉右侧空格/tab/
void trim(string& s);//去掉两侧空格/tab/
void trim_accordingly(string& s, const string& trim_type);//根据trim_type决定调用以上三个函数中的哪个

void change_line_to_hex(const char* s, bool eof);//把一行内容转hex，eof决定是否打印<EOF>
string to_lower(const string& str);//把字符串转为小写
bool compare_strings(const string& s1, const string& s2, bool is_case_sensitive = true);//比较两字符串是否一样，考虑大小写敏感和不敏感两种情况
bool is_digits(const string& s);//判断字符串是否全为数字
