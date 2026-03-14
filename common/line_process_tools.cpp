/* 2452487 信安 胡中芃 */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include "../include/line_process_tools.h" 
using namespace std;

/***************************************************************************
  函数名称：trim_left
  功    能：去除左侧空格/tab/\r
  输入参数：string& s：要处理的字符串
  返 回 值：无
  说    明：find_first_not_of等一系列函数如果没找到返回string::npos，该标识符实际上就是INT_MAX的值，使用方法同INT_MAX，后续同
***************************************************************************/
void trim_left(string& s)
{
    int first = static_cast<int>(s.find_first_not_of(" \t\r"));//查找第一个非空白字符的位置（非空格、tab、\r）
    if (first == string::npos) {//string::nops表示未找到，说明字符串为空
        s.clear();
        return;
    }
    if (first > 0) 
        s.erase(0, first);
}

/***************************************************************************
  函数名称：trim_right
  功    能：去除右侧空格/tab/\r
  输入参数：string& s：要处理的字符串
  返 回 值：无
  说    明：使用find_last_not_of查找最后一个非空白字符并去除右侧空白字符
***************************************************************************/
void trim_right(string& s)
{
    if (s.find_last_not_of(" \t\r") == string::npos) {//查找最后一个非空白字符的位置（非空格、tab、\r）
        s.clear();
        return;
    }
	s.erase(s.find_last_not_of(" \t\r") + 1);//去掉右侧空白字符
}

/***************************************************************************
  函数名称：trim
  功    能：去掉两侧的空格、tab
  输入参数：string& s：要处理的字符串
  返 回 值：无
  说    明：trim_left+trim_right
***************************************************************************/
void trim(string& s)
{
    trim_left(s);
	trim_right(s);
}

/***************************************************************************
  函数名称：trim_accordingly
  功    能：根据第二参处理空格、tab
  输入参数：string& s：要处理的字符串，const string& trim_type：处理方式，left/right/all/none，其他参数打印异常
  返 回 值：无
  说    明：也可用switch-case结合
  enum class Trim_Type {
	TRIM_NONE = 0,
	TRIM_LEFT,
	TRIM_RIGHT,
	TRIM_ALL
};，但是要多转换函数，直接用if-else更简洁
***************************************************************************/
void trim_accordingly(string& s, const string& trim_type)
{
    if (trim_type == "left")
        trim_left(s);
    else if (trim_type == "right")
        trim_right(s);
    else if (trim_type == "all")
        trim(s);
    else if (trim_type == "none")
        ;
    else 
		cout << "无法识别该trim类型:" << trim_type << "，不做任何处理." << endl;
}

/***************************************************************************
  函数名称：change_line_to_hex
  功    能：把一行内容转hex
  输入参数：const char* s：行内容；bool eof：是否文件末尾，用于在外部决定是否打印<EOF>
  返 回 值：无
  说    明：size_t类型和strlen返回类型一致，避免隐式转换warning
***************************************************************************/
void change_line_to_hex(const char* s, bool eof)
{
    size_t len = 0;
    if (s != nullptr) 
        len = strlen(s);
    
    vector<unsigned char> all_bytes;// 收集原始字节
    all_bytes.reserve(len + 2);
    for (size_t i = 0; i < len; ++i)
        all_bytes.push_back(static_cast<unsigned char>(s[i]));
    
    if (eof) {// 根据 eof 决定尾部应追加的字节（统一追加 LF 或 EOF）        
        all_bytes.push_back(0x1A);// 仅追加 EOF 标志 0x1A
    }
    else {      
        all_bytes.push_back(0x0A); // 追加 LF（0x0A），如果原始已有CR (0x0D)，则合并后显示为 0d 0a
    }

    // 按行打印 hex
    for (size_t offset = 0; offset < all_bytes.size(); offset += bytes_per_line) {
        cout << hex << setfill('0') << setw(8) << offset << " : ";
        string ascii_chars;
        for (int j = 0; j < bytes_per_line; ++j) {
            if (j == bytes_per_line / 2)
                cout << ((offset + bytes_per_line / 2 < all_bytes.size()) ? "- " : "  ");
            if (offset + j < all_bytes.size()) {
                unsigned char ch = all_bytes[offset + j];
                cout << hex << setw(2) << setfill('0') << (int)ch << " ";
                ascii_chars += (ch >= 33 && ch <= 126) ? static_cast<char>(ch) : '.';
            }
            else {
                cout << "   ";
            }
        }
        cout << " " << ascii_chars << endl;
    }
   
    cout << dec; // 恢复为十进制输出（不额外输出 <EOF> / <EMPTY>，HEX 区以字节表示尾部）
}

/***************************************************************************
  函数名称：to_lower
  功    能：把字符串转小写
  输入参数：const string& str：要处理的字符串
  返 回 值：处理后的字符串
  说    明：
***************************************************************************/
string to_lower(const string& str) 
{
    string lower_str = str;
    for (int i = 0; i < static_cast<int>(lower_str.length()); ++i) {
        if (lower_str[i] >= 'A' && lower_str[i] <= 'Z')
            lower_str[i] = lower_str[i] + ('a' - 'A');
    }
    return lower_str;
}

/***************************************************************************
  函数名称：compare_strings
  功    能：比较两字符串是否一样，分别考虑大小写敏感和不敏感两种情况
  输入参数：const string& s1、const string& s2：两字符串，bool is_case_sensitive=true：是否大小写敏感，默认分大小写
  返 回 值：处理后的字符串
  说    明：如果is_case_sensitive为true，则大小写敏感比较，直接返回s1==s2结果，否则返回转小写后的比较
***************************************************************************/
bool compare_strings(const string& s1, const string& s2, bool is_case_sensitive)
{
    if (is_case_sensitive)
        return s1 == s2;
    else
        return to_lower(s1) == to_lower(s2);
}

/***************************************************************************
  函数名称：is_digits
  功    能：判断字符串是否全为数字
  输入参数：s - 待检查字符串
  返 回 值：true - 全数字；false - 非全数字或空串
  说    明：用于学号/课号格式校验
***************************************************************************/
bool is_digits(const string& s)
{
    if (s.empty())
        return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (!isdigit((unsigned char)s[i]))
            return false;
    }
    return true;
}