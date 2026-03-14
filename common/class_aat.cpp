/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include "../include/class_aat.h"
//如有必要，可以加入其它头文件
using namespace std;

#if !ENABLE_LIB_COMMON_TOOLS //不使用lib才有效

/* ---------------------------------------------------------------
	 允许加入其它需要static函数（内部工具用）
   ---------------------------------------------------------------- */
/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：判断IP地址合法性
 ***************************************************************************/
static bool is_ipaddr_valid(const char* const ipstr)
{
	int dot_count = 0;
	for (int i = 0; ipstr[i]; i++) {
		if (!(ipstr[i] == '.' || (ipstr[i] >= '0' && ipstr[i] <= '9')))
			return false; // 包含非法字符
		if (ipstr[i] == '.')
			dot_count++;
	}
	if (dot_count != 3)
		return false; // 点的数量不等于3，非法，先检查点的数量，避免数组越界

	int dot_index[3] = { 0 };
	for (int i = 0, j = 0; ipstr[i]; i++) {
		if (ipstr[i] == '.') {
			dot_index[j] = i;
			j++;
		}
	}
	if (dot_index[0] == 0 || dot_index[2] == (int)strlen(ipstr) - 1)
		return false; // 点在开头或结尾，也非法
	for (int i = 0; i < 2; i++) {
		if (dot_index[i + 1] - dot_index[i] == 1)
			return false; // 说明有连续的点，比如192..168.1.1，也非法
	}

	int ip_number[4] = { 0 };
	ip_number[0] = atoi(ipstr); // 从开头直到第一个点停止读取
	for (int i = 0; i < 3; i++)
		ip_number[i + 1] = atoi(&ipstr[dot_index[i] + 1]); // 每一段从点直到碰到下一个点停止读取
	for (int i = 0; i < 4; i++) {
		if (ip_number[i] < 0 || ip_number[i] > 255)
			return false;//数字超出合法范围，则非法
	}

	return true; 
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：判断是否是合法整数，第一位可以是±/数字，后续位均为数字
 ***************************************************************************/
static bool is_valid_int(const char* s)
{
	if (!s || !*s) 
		return false;//为空直接返回false
	int i = 0;
	if (s[i] == '+' || s[i] == '-') {//首位允许±
		if (!s[i + 1])
			return false;//单独+/-
		i++;
	}
	for (; s[i]; i++) {
		if (s[i] < '0' || s[i] > '9') 
			return false;//非数字
	}
	return true;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：判断是否是合法小数，第一位可以是±/数字，可以出现一个.，其余位均为数字
 ***************************************************************************/
static bool is_valid_double(const char* s)
{
	if (!s || !*s)
		return false;
	int i = 0;
	if (s[i] == '+' || s[i] == '-') {
		if (!s[i + 1]) 
			return false;
		i++;
	}//以上同is_valid_int的判断
	int dot = 0;
	for (; s[i]; i++) {
		if (s[i] == '.') {
			dot++;
			if (dot > 1) 
				return false;//小数点多于一个
		}
		else if (s[i] < '0' || s[i] > '9')
			return false;
	}
	return true;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：把ip地址字符串转换为unsigned int
 ***************************************************************************/
static unsigned int change_ip_to_uint(const char* const ipstr)
{
	unsigned int ip = 0;
	int dot_index[3] = { 0 };
	for (int i = 0, j = 0; ipstr[i]; i++) {
		if (ipstr[i] == '.') {
			dot_index[j] = i;
			j++;
		}
	}
	ip |= (atoi(ipstr) << 24);
	ip |= (atoi(&ipstr[dot_index[0] + 1]) << 16);
	ip |= (atoi(&ipstr[dot_index[1] + 1]) << 8);
	ip |= (atoi(&ipstr[dot_index[2] + 1]));
	return ip;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：打印int集合，没到INVALID_INT_VALUE_OF_SET就打印下一个元素和分隔符
 ***************************************************************************/
static void print_int_set(const int* p)
{
	for (; *p != INVALID_INT_VALUE_OF_SET; ) {
		cout << *p++;
		cout << (*p == INVALID_INT_VALUE_OF_SET ? "]" : "/");
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：打印double集合，没到INVALID_DOUBLE_VALUE_OF_SET就打印下一个元素和分隔符
 ***************************************************************************/
static void print_double_set(const double* p)
{
	for (; *p != INVALID_DOUBLE_VALUE_OF_SET; ) {
		cout << *p++;
		cout << (*p == INVALID_DOUBLE_VALUE_OF_SET ? "]" : "/");
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：打印string集合，p->empty()非空就打印下一个元素和分隔符
 ***************************************************************************/
static void print_string_set(const string* p)
{
	for (; !p->empty(); ) {
		cout << *p++;
		cout << (p->empty() ? "]" : "/");
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：画分割线，画length个等号
 ***************************************************************************/
static void print_segementation_with_equal_sign(int length)
{
	for (int i = 0; i < length; i++)
		cout << "=";
	cout << endl;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：null，全部清除，用初始化表形式缺省的复制构造函数防止智能提示，pad为空即可
 ***************************************************************************/
args_analyse_tools::args_analyse_tools()
	: args_name(),
	extargs_type(ST_EXTARGS_TYPE::null),
	extargs_num(0),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{} 
{
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：boolean
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const bool def)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(def),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：int_with_default、int_with_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const int def, const int _min, const int _max)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(def),
	extargs_int_min(_min),
	extargs_int_max(_max),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	if (extargs_int_default < extargs_int_min || extargs_int_default > extargs_int_max)
		extargs_int_default = extargs_int_min;
	extargs_int_value = extargs_int_default;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：int_with_set_default、int_with_set_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const int def_of_set_pos, const int* const set)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(const_cast<int*>(set)),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	int size_of_set = 0;
	while (set[size_of_set] != INVALID_INT_VALUE_OF_SET)
		size_of_set++;
	const int p = (def_of_set_pos >= 0 && def_of_set_pos < size_of_set) ? def_of_set_pos : 0;
	extargs_int_default = set[p];
	extargs_int_value = extargs_int_default;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：str、ipaddr_with_default、ipaddr_with_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const string def)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	switch (type)
	{
		case ST_EXTARGS_TYPE::str:
			extargs_string_default = def;
			extargs_string_value = def;
			break;
		case ST_EXTARGS_TYPE::ipaddr_with_default:
		{
			u_int v = 0;
			if (!def.empty() && is_ipaddr_valid(def.c_str()))
				v = change_ip_to_uint(def.c_str());
			extargs_ipaddr_default = v;
			extargs_ipaddr_value = v;
			break;
		}
		case ST_EXTARGS_TYPE::ipaddr_with_error:
			extargs_ipaddr_default = 0;
			extargs_ipaddr_value = 0;
			break;
		default:
			extargs_string_default = def;
			extargs_string_value = def;
			break;
	}
	if (type != ST_EXTARGS_TYPE::str) {
		extargs_string_value.clear();
	}
	if (type != ST_EXTARGS_TYPE::ipaddr_with_default && type != ST_EXTARGS_TYPE::ipaddr_with_error) {
		extargs_ipaddr_default = 0;
		extargs_ipaddr_value = 0;
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：str_with_set_default、str_with_set_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const int def_of_set_pos, const string* const set)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(const_cast<string*>(set)),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	int size_of_set = 0;
	while (!set[size_of_set].empty())
		size_of_set++;
	const int p = (def_of_set_pos >= 0 && def_of_set_pos < size_of_set) ? def_of_set_pos : 0;
	extargs_string_default = set[p];
	extargs_string_value = extargs_string_default;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：double_with_default、double_with_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const double def, const double _min, const double _max)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(def),
	extargs_double_min(_min),
	extargs_double_max(_max),
	extargs_double_set(nullptr),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(def),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	if (def < extargs_double_min || def > extargs_double_max)
		extargs_double_default = extargs_double_min;
	extargs_double_value = extargs_double_default;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：double_with_set_default、double_with_set_error
 ***************************************************************************/
args_analyse_tools::args_analyse_tools(const char* arg_name, const enum ST_EXTARGS_TYPE type, const int ext_num, const int def_of_set_pos, const double* const set)
	: args_name(arg_name ? arg_name : ""),
	extargs_type(type),
	extargs_num(ext_num),
	extargs_bool_default(false),
	extargs_int_default(0),
	extargs_int_min(0),
	extargs_int_max(0),
	extargs_int_set(nullptr),
	extargs_double_default(0.0),
	extargs_double_min(0.0),
	extargs_double_max(0.0),
	extargs_double_set(const_cast<double*>(set)),
	extargs_string_default(),
	extargs_string_set(nullptr),
	extargs_ipaddr_default(0),
	args_existed(0),
	extargs_int_value(0),
	extargs_double_value(0.0),
	extargs_string_value(),
	extargs_ipaddr_value(0),
	pad{}
{
	int size_of_set = 0;
	while (set[size_of_set] != INVALID_DOUBLE_VALUE_OF_SET)
		size_of_set++;
	const int p = (def_of_set_pos >= 0 && def_of_set_pos < size_of_set) ? def_of_set_pos : 0;
	extargs_double_default = set[p];
	extargs_double_value = extargs_double_default;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：没有动态分配内存，不需要释放
 ***************************************************************************/
args_analyse_tools::~args_analyse_tools()
{
	// 集合指针均为外部数组，不负责释放
}

/* ---------------------------------------------------------------
	 允许AAT中自定义成员函数的实现（private）
   ---------------------------------------------------------------- */

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：已实现，不要动
 ***************************************************************************/
const string args_analyse_tools::get_name() const
{
	return this->args_name;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：加!!后，只能是0/1
			已实现，不要动
 ***************************************************************************/
const int args_analyse_tools::existed() const
{
	return !!this->args_existed;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：已实现，不要动
 ***************************************************************************/
const int args_analyse_tools::get_int() const
{
	return this->extargs_int_value;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：已实现，不要动
 ***************************************************************************/
const double args_analyse_tools::get_double() const
{
	return this->extargs_double_value;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：已实现，不要动
 ***************************************************************************/
const string args_analyse_tools::get_string() const
{
	return this->extargs_string_value;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：已实现，不要动
 ***************************************************************************/
const unsigned int args_analyse_tools::get_ipaddr() const
{
	return this->extargs_ipaddr_value;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：将 extargs_ipaddr_value 的值从 0x7f000001 转为 "127.0.0.1"
 ***************************************************************************/
const string args_analyse_tools::get_str_ipaddr() const
{
	ostringstream fout;
	fout << ((extargs_ipaddr_value >> 24) & 0xFF) << "." 
		<< ((extargs_ipaddr_value >> 16) & 0xFF) << "." 
		<< ((extargs_ipaddr_value >> 8) & 0xFF) << "."
		<< (extargs_ipaddr_value & 0xFF);

	return fout.str();
}


/***************************************************************************
  函数名称：
  功    能：
  输入参数：follow_up_args：是否有后续参数
			0  ：无后续参数
			1  ：有后续参数
  返 回 值：
  说    明：友元函数
***************************************************************************/
int args_analyse_process(const int argc, const char* const *const argv, args_analyse_tools* const args, const int follow_up_args)
{
	for (int i = 1; i < argc; ) {		
		if (!(argv[i][0] == '-' && argv[i][1] == '-') || // 不是--开头
			(argv[i][0] == '-' && argv[i][1] == '-' && strlen(argv[i]) == 2)) {	//只有--
			if (follow_up_args == 1)
				return i; // 固定参数从这里开始
			else {
				cout << "参数[" << argv[i] << "]格式非法(不是--开头的有效内容)." << endl;
				return -1;
			}
		}
		else {
			int index = -1;
			for (int k = 0; ; k++) { //在 args[]遍历查找这个选项
				if (args[k].extargs_type == ST_EXTARGS_TYPE::null)
					break;
				if (args[k].args_name == argv[i]) {
					index = k;//找到了index表示下标
					break;
				}
			}
			if (index < 0) {
				cout << "参数[" << argv[i] << "]非法." << endl;//没找到打印非法
				return -1;
			}		
			if (args[index].args_existed) {
				cout << "参数[" << argv[i] << "]重复." << endl;
				return -1;
			}
			args[index].args_existed = 1;//置为1，如果下次再出现，那么按要求不覆盖而是检出错误
			if (args[index].extargs_num == 0) {	
				i += 1; // boolean，增加一个参数并继续下一轮循环
				continue;
			}
			else if (i + 1 >= argc || (argv[i + 1][0] == '-' && argv[i + 1][1] == '-')) {
				if (args[index].extargs_type == ST_EXTARGS_TYPE::int_with_default||
					args[index].extargs_type == ST_EXTARGS_TYPE::int_with_error) {
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:int, 范围["
						<< args[index].extargs_int_min << ".." << args[index].extargs_int_max
						<< "] 缺省:" << args[index].extargs_int_default << ")" << endl;
				}
				else if (args[index].extargs_type == ST_EXTARGS_TYPE::double_with_default
					|| args[index].extargs_type == ST_EXTARGS_TYPE::double_with_error) {
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:double, 范围["
						<< args[index].extargs_double_min << ".." << args[index].extargs_double_max
						<< "] 缺省:" << args[index].extargs_double_default << ")" << endl;
				}
				else if (args[index].extargs_type == ST_EXTARGS_TYPE::str) {
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:string"
						<< args[index].extargs_string_default << ")" << endl;
				}
				else if (args[index].extargs_type == ST_EXTARGS_TYPE::ipaddr_with_default ||
					args[index].extargs_type == ST_EXTARGS_TYPE::ipaddr_with_error) {
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:IP地址 缺省:"
						<< args[index].get_str_ipaddr() << ")" << endl;
				}
				else if (args[index].extargs_type == ST_EXTARGS_TYPE::int_with_set_default ||
					args[index].extargs_type == ST_EXTARGS_TYPE::int_with_set_error) {
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:int, 可取值[";
					print_int_set(args[index].extargs_int_set); // 输出 0/90/180/270]
					if (args[index].extargs_type == ST_EXTARGS_TYPE::int_with_set_default)
						cout << " 缺省:" << args[index].extargs_int_default;
					cout << ")" << endl;
				}
				else if (args[index].extargs_type == ST_EXTARGS_TYPE::double_with_set_default ||
                    args[index].extargs_type == ST_EXTARGS_TYPE::double_with_set_error) {
                    cout << "参数[" << argv[i] << "]的附加参数不足. (类型:double, 可取值[";
                    print_double_set(args[index].extargs_double_set);
                    if (args[index].extargs_type == ST_EXTARGS_TYPE::double_with_set_default)
                        cout << " 缺省:" << args[index].extargs_double_default;
                    cout << ")" << endl;
                    }

				else if (args[index].extargs_type == ST_EXTARGS_TYPE::str_with_set_default ||
					args[index].extargs_type == ST_EXTARGS_TYPE::str_with_set_error) {
					string setstr;
					if (args[index].extargs_string_set) {
						for (int p = 0; !args[index].extargs_string_set[p].empty(); ++p) {
							if (!setstr.empty())
								setstr += "/";
							setstr += args[index].extargs_string_set[p];
						}
					}
					cout << "参数[" << argv[i] << "]的附加参数不足. (类型:string, 可取值[" << setstr << "]";
					if (args[index].extargs_type == ST_EXTARGS_TYPE::str_with_set_default) {
						cout << " 缺省:" << (args[index].extargs_string_default.empty() ? "/" : args[index].extargs_string_default);
					}
					cout << ")" << endl;
				}
				else
					cout << "参数[" << argv[i] << "]的附加参数不足." << endl;
				
				return -1;
			}//end of else if，一个参数的时候检查缺值

			// 到这里还没有return -1，说明有1个附加参数，下面将按照参数的不同类型进行具体的解析操作

			const char* val = argv[i + 1];//取出该附加参数
			ST_EXTARGS_TYPE t = args[index].extargs_type;//类型取名为t

			/* 分类型解析与合法性 */
			if (t == ST_EXTARGS_TYPE::int_with_default || t == ST_EXTARGS_TYPE::int_with_error) {
				if (!is_valid_int(val)) {
					cout << "参数[" << argv[i] << "]的附加参数不是整数. (类型:int, 可取值[" << args[index].extargs_int_min
						<< ".." << args[index].extargs_int_max << "]";
					if (t == ST_EXTARGS_TYPE::int_with_default)
						cout << " 缺省:" << args[index].extargs_int_default;
					cout << ")" << endl;
					return -1;
				}
				int v = atoi(val);
				if (v < args[index].extargs_int_min || v > args[index].extargs_int_max) {
					if (t == ST_EXTARGS_TYPE::int_with_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << v << ")非法. (类型:int, 可取值[" << args[index].extargs_int_min
							<< ".." << args[index].extargs_int_max << "])" << endl;
						return -1;
					}
					else {
						v = args[index].extargs_int_default;
					}
				}
				args[index].extargs_int_value = v;
			}
			else if (t == ST_EXTARGS_TYPE::int_with_set_default || t == ST_EXTARGS_TYPE::int_with_set_error) {
				if (!is_valid_int(val)) {
					cout << "参数[" << argv[i] << "]的附加参数不是整数. (类型:int, 范围[";
					print_int_set(args[index].extargs_int_set);
					if (t == ST_EXTARGS_TYPE::int_with_set_default)
						cout << " 缺省:" << args[index].extargs_int_default;
					cout << ")" << endl;
					return -1;
				}
				int v = atoi(val);
				bool in_set = false;//标记是否在集合中
				for (const int* p = args[index].extargs_int_set; *p != INVALID_INT_VALUE_OF_SET; ++p)
					if (*p == v) {
						in_set = true; 
						break; 
					}

				if (!in_set) {
					if (t == ST_EXTARGS_TYPE::int_with_set_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << v << ")非法. (类型:int, 可取值[";
						print_int_set(args[index].extargs_int_set);
						cout << ")" << endl;
						return -1;
					}
					else {
						v = args[index].extargs_int_default;
					}
				}
				args[index].extargs_int_value = v;
			}
			else if (t == ST_EXTARGS_TYPE::double_with_default || t == ST_EXTARGS_TYPE::double_with_error) {
				if (!is_valid_double(val)) {
					cout << "参数[" << argv[i] << "]的附加参数不是浮点数. (类型:double, 范围[" << args[index].extargs_double_min
						<< ".." << args[index].extargs_double_max << "]";
					if (t == ST_EXTARGS_TYPE::double_with_default)
						cout << "缺省:" << args[index].extargs_double_default;
					cout << ")" << endl;
					return -1;
				}
				double dv = atof(val);
				if (dv < args[index].extargs_double_min - DOUBLE_DELTA || dv > args[index].extargs_double_max + DOUBLE_DELTA) {
					if (t == ST_EXTARGS_TYPE::double_with_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << dv << ")非法. (类型:double, 范围[" << args[index].extargs_double_min
							<< ".." << args[index].extargs_double_max << "])" << endl;
						return -1;
					}
					else {
						dv = args[index].extargs_double_default;
					}
				}
				args[index].extargs_double_value = dv;
			}
			else if (t == ST_EXTARGS_TYPE::double_with_set_default || t == ST_EXTARGS_TYPE::double_with_set_error) {
				if (!is_valid_double(val)) {
					cout << "参数[" << argv[i] << "]的附加参数不是浮点数. (类型:double, 可取值[";
					print_double_set(args[index].extargs_double_set);
					if (t == ST_EXTARGS_TYPE::double_with_set_default)
						cout << "缺省:" << args[index].extargs_double_default;
					cout << ")" << endl;
					return -1;
				}
				double dv = atof(val);
				bool in_set = false;
				for (const double* p = args[index].extargs_double_set; *p != INVALID_DOUBLE_VALUE_OF_SET; ++p)
					if (fabs(*p - dv) < DOUBLE_DELTA) {//小于1e-6即认为相等
						in_set = true;
						break;
					}

				if (!in_set) {
					if (t == ST_EXTARGS_TYPE::double_with_set_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << dv << ")非法. (类型:double, 可取值[";
						print_double_set(args[index].extargs_double_set);
						cout << ")" << endl;
						return -1;
					}
					else {
						dv = args[index].extargs_double_default;
					}
				}
				args[index].extargs_double_value = dv;
			}
			else if (t == ST_EXTARGS_TYPE::str) {	
				args[index].extargs_string_value = val;// 直接赋值
			}
			else if (t == ST_EXTARGS_TYPE::str_with_set_default || t == ST_EXTARGS_TYPE::str_with_set_error) {
				string sval = val;
				bool in_set = false;
				for (const string* p = args[index].extargs_string_set; !p->empty(); ++p)
					if (*p == sval) { 
						in_set = true;
						break; 
					}
				if (!in_set) {
					if (t == ST_EXTARGS_TYPE::str_with_set_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << sval << ")非法. (类型:string, 可取值[";
						print_string_set(args[index].extargs_string_set);
						cout << ")" << endl;
						return -1;
					}
					else {
						sval = args[index].extargs_string_default;
					}
				}
				args[index].extargs_string_value = sval;
			}
			else if (t == ST_EXTARGS_TYPE::ipaddr_with_default || t == ST_EXTARGS_TYPE::ipaddr_with_error) {
				if (!is_ipaddr_valid(val)) {
					if (t == ST_EXTARGS_TYPE::ipaddr_with_error) {
						cout << "参数[" << argv[i] << "]的附加参数值(" << val << ")非法. (类型:IP地址)" << endl;
						return -1;
					}
					else {
						args[index].extargs_ipaddr_value = args[index].extargs_ipaddr_default;
					}
				}
				else {
					args[index].extargs_ipaddr_value = change_ip_to_uint(val);
				}
			}
			i += 2;//有1个附加值增加2个参数并继续下一轮循环
		}			
	}	
	
	return argc;//如果没有固定参数，返回 argc
}


/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：友元函数
***************************************************************************/
int args_analyse_print(const args_analyse_tools* const args)
{
	int row = 0;
	while (args[row].extargs_type != ST_EXTARGS_TYPE::null && !args[row].args_name.empty())
		row++;//类型为空（或者NULL也考虑进去）退出循环，则row为总行数

	const char* header[6] = {"name","type","default","exists","value","range/set" };//表头
	int len[6];//各列宽度
	for (int i = 0; i < 6; i++)
		len[i] = (int)strlen(header[i]);

	const char* type_names[] = {
		"", "Bool", "IntWithDefault", "IntWithError", "IntSETWithDefault", "IntSETWithError",
		"DoubleWithDefault", "DoubleWithError","DoubleSETWithDefault", "DoubleSETWithError","String", 
		"StringSETWithDefault", "StringSETWithError","IPAddrWithDefault", "IPAddrWithError"
	};//各种情况下需要打印的类型名称

	//第一次遍历，只计算各列最大宽度
	for (int i = 0; i < row; i++) {	
		//下面更新name列最大宽度
		int w = (int)args[i].args_name.size();//string类size函数功能：返回字符串长度
		if (w > len[0]) 
			len[0] = w;
		//下面更新type列最大宽度
		int tindex = (int)args[i].extargs_type;//tindex为枚举类的下标，缩短名称
		const char* tname = (tindex >= 0 && tindex < (int)(sizeof(type_names) / sizeof(type_names[0]))) ? type_names[tindex] : "";
		w = (int)strlen(tname);
		if (w > len[1]) 
			len[1] = w;
		//下面更新default列最大宽度
		const char* type_str = tname;
		string defv = "/";
		if (strstr(type_str, "Error") == NULL) { //非Error型才显示真实default 
			if (strstr(type_str, "Int") != NULL) 
				defv = to_string(args[i].extargs_int_default);//to_string函数功能：把整数转换为字符串
			else if (strstr(type_str, "Double") != NULL) 
				defv = to_string(args[i].extargs_double_default);		
			else if (strstr(type_str, "String") != NULL) {
				if (!args[i].extargs_string_default.empty())
					defv = args[i].extargs_string_default;	
			}
			else if (strstr(type_str, "Bool") != NULL) 
				defv = args[i].extargs_bool_default ? "true" : "false";	
			else if (strstr(type_str, "IP") != NULL) {
				ostringstream out;
				out << ((args[i].extargs_ipaddr_default >> 24) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_default >> 16) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_default >> 8) & 0xFF) << "."
					<< (args[i].extargs_ipaddr_default & 0xFF);
				defv = out.str();
			}
		}
		w = (int)defv.size();
		if (w > len[2])
			len[2] = w;
		//下面更新exists列最大宽度
		string existv = to_string(args[i].args_existed);
		w = (int)existv.size();
		if (w > len[3]) 
			len[3] = w;
		//下面更新value列最大宽度
		string valv = "/";
		if (args[i].args_existed) {
			if (strstr(type_str, "Int") != NULL) 
				valv = to_string(args[i].extargs_int_value);
			else if (strstr(type_str, "Double") != NULL) 
				valv = to_string(args[i].extargs_double_value);			
			else if (strstr(type_str, "String") != NULL) 
				valv = args[i].extargs_string_value.empty() ? "/" : args[i].extargs_string_value;			
			else if (strstr(type_str, "Bool") != NULL) 
				valv = args[i].args_existed ? "true" : "false";			
			else if (strstr(type_str, "IP") != NULL) {
				ostringstream out;
				out << ((args[i].extargs_ipaddr_value >> 24) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_value >> 16) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_value >> 8) & 0xFF) << "."
					<< (args[i].extargs_ipaddr_value & 0xFF);
				valv = out.str();
			}
		}
		w = (int)valv.size();
		if (w > len[4]) 
			len[4] = w;
		//下面更新range/set列最大宽度 
		string rangev = "/";
		if (strstr(type_str, "Int") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_int_set) {
					rangev.clear();
					int p = 0;
					while (args[i].extargs_int_set[p] != INVALID_INT_VALUE_OF_SET) {
						if (p)
							rangev += "/";
						rangev += to_string(args[i].extargs_int_set[p]);
						p++;
					}
					if (rangev.empty()) 
						rangev = "/";
				}
			}
			else {
				rangev = "[" + to_string(args[i].extargs_int_min) + ".." + to_string(args[i].extargs_int_max) + "]";
			}
		}
		else if (strstr(type_str, "Double") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_double_set) {
					rangev.clear();
					int p = 0;
					while (args[i].extargs_double_set[p] != INVALID_DOUBLE_VALUE_OF_SET) {
						if (p)
							rangev += "/";
						ostringstream out;
						out.setf(ios::fixed);
						out << setprecision(6) << args[i].extargs_double_set[p];
						rangev += out.str();
						p++;
					}
					if (rangev.empty()) 
						rangev = "/";
				}
			}
			else {
				ostringstream out;
				out.setf(ios::fixed);
				out << "[" << setprecision(6) << args[i].extargs_double_min
					<< ".." << setprecision(6) << args[i].extargs_double_max << "]";
				rangev = out.str();
			}
		}
		else if (strstr(type_str, "String") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_string_set) {
					rangev.clear();
					int p = 0;
					while (!args[i].extargs_string_set[p].empty()) {
						if (p)
							rangev += "/";
						rangev += args[i].extargs_string_set[p];
						p++;
					}
					if (rangev.empty()) 
						rangev = "/";
				}
			}
			else 
				rangev = "/";			
		}
		else if (strstr(type_str, "Bool") != NULL) 
			rangev = "/";
		else if (strstr(type_str, "IP") != NULL) 
			rangev = "/";	
		w = (int)rangev.size();
		if (w > len[5])
			len[5] = w;
	}
	
	int total = 0; 
	for (int c = 0; c < 6; c++) 
		total += (len[c] + 1); // 每列内容+一个前导空格
	total++;//最后额外多打印一个空格
	/*
	for (int c = 0; c < 6; c++) {
		if (len[c] != strlen(header[c])+1) {
			len[c]--;
		}	
	}
	*/

	print_segementation_with_equal_sign(total);//打印等号上分隔线
	for (int c = 0; c < 6; c++)
		cout << " " << setiosflags(ios::left) << setw(len[c]) << header[c];//打印表头
	cout << endl;
	print_segementation_with_equal_sign(total);//第二条分隔线

	//cout << len[3]<<endl;
	// 
	//第二次遍历，打印每一行
	for (int i = 0; i < row; i++) {
		int tindex = (int)args[i].extargs_type;
		const char* type_str = (tindex >= 0 && tindex < (int)(sizeof(type_names) / sizeof(type_names[0]))) ? type_names[tindex] : "";
        //下面打印default列
		string defv = "/";
		if (strstr(type_str, "Error") == NULL) {
			if (strstr(type_str, "Int") != NULL) 
				defv = to_string(args[i].extargs_int_default);
			else if (strstr(type_str, "Double") != NULL) {
				ostringstream out;
				out.setf(ios::fixed); 
				out << setprecision(6) << args[i].extargs_double_default;
				defv = out.str();
			}
			else if (strstr(type_str, "String") != NULL) {
				if (!args[i].extargs_string_default.empty()) 
					defv = args[i].extargs_string_default;
			}
			else if (strstr(type_str, "Bool") != NULL) 
				defv = args[i].extargs_bool_default ? "true" : "false";
			else if (strstr(type_str, "IP") != NULL) {
				ostringstream out;
				out << ((args[i].extargs_ipaddr_default >> 24) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_default >> 16) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_default >> 8) & 0xFF) << "."
					<< (args[i].extargs_ipaddr_default & 0xFF);
				defv = out.str();
			}
		}
	
		//下面是value列处理
		string valv = "/";
		if (args[i].args_existed) {
			if (strstr(type_str, "Int") != NULL) 
				valv = to_string(args[i].extargs_int_value);
			else if (strstr(type_str, "Double") != NULL) {
				ostringstream out; 
				out.setf(ios::fixed); 
				out << setprecision(6) << args[i].extargs_double_value;
				valv = out.str();
			}
			else if (strstr(type_str, "String") != NULL) 
				valv = args[i].extargs_string_value.empty() ? "/" : args[i].extargs_string_value;
			else if (strstr(type_str, "Bool") != NULL) 
				valv = args[i].args_existed ? "true" : "false";
			else if (strstr(type_str, "IP") != NULL) {
				ostringstream out;
				out << ((args[i].extargs_ipaddr_value >> 24) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_value >> 16) & 0xFF) << "."
					<< ((args[i].extargs_ipaddr_value >> 8) & 0xFF) << "."
					<< (args[i].extargs_ipaddr_value & 0xFF);
				valv = out.str();
			}
		}

		//下面打印range/set列
		string rangev = "/";
		if (strstr(type_str, "Int") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_int_set) {
					rangev.clear();
					int p = 0;
					while (args[i].extargs_int_set[p] != INVALID_INT_VALUE_OF_SET) {
						if (p) 
							rangev += "/";
						rangev += to_string(args[i].extargs_int_set[p]);
						p++;
					}
					if (rangev.empty()) 
						rangev = "/";
				}
			}
			else {
				rangev = "[" + to_string(args[i].extargs_int_min) + ".." + to_string(args[i].extargs_int_max) + "]";
			}
		}
		else if (strstr(type_str, "Double") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_double_set) {
					rangev.clear();
					int p = 0;
					while (args[i].extargs_double_set[p] != INVALID_DOUBLE_VALUE_OF_SET) {
						if (p)
							rangev += "/";
						ostringstream out;
						out.setf(ios::fixed);
						out << setprecision(6) << args[i].extargs_double_set[p];
						rangev += out.str();
						p++;
					}
					if (rangev.empty())
						rangev = "/";
				}
			}
			else {
				ostringstream out;
				out.setf(ios::fixed);
				out << "[" << setprecision(6) << args[i].extargs_double_min
					<< ".." << setprecision(6) << args[i].extargs_double_max << "]";
				rangev = out.str();
			}
		}
		else if (strstr(type_str, "String") != NULL) {
			if (strstr(type_str, "SET") != NULL) {
				if (args[i].extargs_string_set) {
					rangev.clear();
					int p = 0;
					while (!args[i].extargs_string_set[p].empty()) {
						if (p)
							rangev += "/";
						rangev += args[i].extargs_string_set[p];
						p++;
					}
					if (rangev.empty())
						rangev = "/";
				}
			}
			else
				rangev = "/";
		}
		else if (strstr(type_str, "Bool") != NULL)
			rangev = "/";
		else if (strstr(type_str, "IP") != NULL)
			rangev = "/";

		cout << " " << setiosflags(ios::left) << setw(len[0]) << args[i].get_name()
			<< " " << setiosflags(ios::left) << setw(len[1]) << type_str
			<< " " << setiosflags(ios::left) << setw(len[2]) << defv
			<< " " << setiosflags(ios::left) << setw(len[3]) << args[i].args_existed
			<< " " << setiosflags(ios::left) << setw(len[4]) << valv
			<< " " << setiosflags(ios::left) << setw(len[5]) << rangev << endl;
	}
	print_segementation_with_equal_sign(total);//末尾分割线
	cout << endl;

	return 0;
}
#endif // !ENABLE_LIB_COMMON_TOOLS
