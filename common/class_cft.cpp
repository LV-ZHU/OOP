/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
/* 添加自己需要的头文件，注意限制 */
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/line_process_tools.h"
#include "../include/class_cft.h"
using namespace std;

/* 给出各种自定义函数的实现（已给出的内容不全） */

/***************************************************************************
  函数名称：release_memory
  功    能：按顺序释放内存
  输入参数：无
  返 回 值：无
  说    明：结构体定义：头文件里有
  struct item_node {
	string item_name;//配置项名称
	string item_value;//配置项值
	item_node* next;//指向下一个配置项的指针
	string original_line;//用于保存原始行内容，以保留格式
};
  struct group_node {
	string group_name;//配置组名称
	item_node* items;//指向该组中第一个配置项的指针
	group_node* next;//指向下一个配置组的指针
  };
***************************************************************************/
void config_file_tools::release_memory() 
{
	group_node* current_group = cfg_list;//current_group指向第一个组
	while (current_group != nullptr) {//只要组不为空循环执行
		item_node* current_item = current_group->items;//current_item指向当前组的第一个项
		while (current_item != nullptr) {//只要项不为空循环执行
			item_node* next_item = current_item->next;//保存下一个项的指针
			delete current_item;//先释放当前项
			current_item = next_item;//current_item指向下一个项，从而循环把该组里面的所有项都释放掉
		}
		group_node* next_group = current_group->next;//建立下一个组的指针
		delete current_group;	//释放当前组
		current_group = next_group;//current_group指向下一个组，从而循环把所有组都释放掉
	}
	cfg_list = nullptr;//最后把cfg_list指向空，表示没有任何组，防止野指针
}

/***************************************************************************
  函数名称：find_group
  功    能：查找指定组名的组，看有没有和group_name一样的组名
  输入参数：const string& group_name：组名，bool is_case_sensitive：是否大小写敏感
  返 回 值：group_node*，找到返回该组指针，找不到返回nullptr
  说    明：
 ***************************************************************************/
group_node* config_file_tools::find_group(const string& group_name, bool is_case_sensitive) const 
{
	group_node* current = cfg_list;//从第一个组开始查找
	while (current != nullptr) {//只要组不为空循环执行
		if (compare_strings(current->group_name, group_name, is_case_sensitive)) 
			return current;//找到了就返回该组指针
		current = current->next;//到这里说明没找到，指向下一个组继续找
	}
	return nullptr;//找不到
}

/***************************************************************************
  函数名称：find_item
  功    能：查找指定组中的指定项，在group中找item_name对应的项
  输入参数：group_node* group：组指针，const string& item_name：项名，bool is_case_sensitive：是否大小写敏感
  返 回 值：item_node*，找到返回该项指针，找不到返回nullptr
  说    明：和find_group区别在于要在group里找，多一参
 ***************************************************************************/
item_node* config_file_tools::find_item(group_node* group, const string& item_name, bool is_case_sensitive) const {
	if (group == nullptr) 
		return nullptr;//组指针为空更不会有项，直接返回空
	item_node* current = group->items;//从该组的第一个项开始查找，后面和find_group一样
	while (current != nullptr) {
		if (compare_strings(current->item_name, item_name, is_case_sensitive)) 
			return current;
		current = current->next;
	}
	return nullptr;
}

/***************************************************************************
  函数名称：parse_file
  功    能：解析文件，建立链表，在构造函数就调用，此函数完成主要工作
  输入参数：无
  返 回 值：无
  说    明：结构体定义：头文件里有
  struct item_node {
	string item_name;//配置项名称
	string item_value;//配置项值
	item_node* next;//指向下一个配置项的指针
	string original_line;//用于保存原始行内容，以保留格式
};
  struct group_node {
	string group_name;//配置组名称
	item_node* items;//指向该组中第一个配置项的指针
	group_node* next;//指向下一个配置组的指针
  };
***************************************************************************/
void config_file_tools::parse_file()
{
	ifstream in(cfgname.c_str());//打开由成员变量cfgname指定的配置文件，.c_str()转换为 C 风格的字符串，以兼容ifstream的构造函数
	if (!in.is_open()) 
		return;

	string one_line_in_file; //存储从文件中读取的每一行内容
	group_node* current_group = nullptr;//定义指向group_node的指针为current_group
	int line_num = 0;//当前在哪一行，初始化为0，用于错误提示

	while (getline(in, one_line_in_file)) {//逐行读取文件内容，存储在one_line_in_file中
		line_num++;
		if (one_line_in_file.length() > MAX_LINE) {//超过最大长度释放后返回
			cout << "非法格式的配置文件，第" << line_num << "行超过最大长度" << MAX_LINE << "." << endl;
			release_memory();
			return;
		}

		string line = one_line_in_file;//该行内容为line
		int comment_pos = line.find_first_of(";#");//查找注释符号位置，单目的;/#
		if (comment_pos != string::npos) 
			line.erase(comment_pos);//.erase函数表示删除从comment_pos位置开始到行尾的内容，删除注释内容
		comment_pos = line.find("//");//查找注释符号位置，双目的//
		if (comment_pos != string::npos) 
			line.erase(comment_pos);//删除注释
		trim(line);//去掉行首尾空白字符
		if (line.empty()) 
			continue;//没东西了就跳过下面步骤继续读下一行

		if (line.front() == '[' && line.back() == ']') {//.front()返回字符串第一个字符，.back()返回字符串最后一个字符，判断是否为组名
			string group_name = line.substr(1, line.length() - 2);//从第一个到总长-2，也就是删除[]留着中间的内容
			trim(group_name);//去掉组名首尾空白字符
			group_node* existing_group = find_group(group_name, true);//区分大小写，查找该组名是否已经存在
			if (existing_group) 
				current_group = existing_group;//存在就把current_group指向该组，后面在已有组的基础上添加内容
			else {
				group_node* new_group = new(nothrow) group_node;
				if (new_group == nullptr) {
					release_memory();
					return;
				}

				new_group->group_name = group_name;
				new_group->items = nullptr;
				new_group->next = nullptr;
						
				if (cfg_list == nullptr) {//cfg_list为空，说明是第一个组
					cfg_list = new_group;//把cfg_list指向该组
					current_group = new_group;//current_group也指向该组	
				}
				else {//不是第一个组
					group_node* tail = cfg_list;//定义tail为头指针
					while (tail->next != nullptr) {	//	指向当前最后一个组
						tail = tail->next;
					}
					tail->next = new_group;//把新组接在最后一个组后面
					current_group = new_group;//current_group为新组
				}
			}
		}
		else {//不是组名，就是项
			if (current_group == nullptr) {//没有组名，说明是简单配置文件
				group_node* simple_group = find_group(SIMPLE_GNAME, true);//查找简单组名""
				if (!simple_group) {
					group_node* p = new(nothrow) group_node;//没有找到就新建一个简单组
					if (p == nullptr) {
						release_memory();
						return;
					}
					p->group_name = SIMPLE_GNAME;//组名设为""
					p->items = nullptr;
					p->next = nullptr;

					simple_group = p;//赋值给simple_group
					if (cfg_list == nullptr) 
						cfg_list = simple_group;//第一个组，把cfg_list指向它
					else {
						group_node* tail = cfg_list;
						while (tail->next != nullptr) {
							tail = tail->next;
						}//同上，tail为头指针，循环后指向最后一个组					
						tail->next = simple_group;//把简单组接在最后一个组后面
					}
				}
				current_group = simple_group;//current_group指向这次操作的组
			}

			string item_name, item_value;//item_name：项目名称，item_value：项目值
			string original_item_line = line;//结构体里的内容，保存原来的整行内容，后续用于get_all_item函数解析
			int separator_pos = string::npos;//初始化为npos，用来标记分隔符的位置

			if (item_separate_character_type == BREAK_CTYPE::Equal) 
				separator_pos = line.find('=');//等号类型找等号
			else //item_separate_character_type == BREAK_CTYPE::Space
				separator_pos = line.find_first_of(" \t");//空格类型找空格或tab
				//查找分隔符，并将行分割为 item_name 和 item_value。
				//如果找不到分隔符，整行被视为 item_name，item_value 为空。

			if (separator_pos != string::npos) {//找到了分隔符
				item_name = line.substr(0, separator_pos);//从0位置开始，提取separator_pos个字符作为item_name
				item_value = line.substr(separator_pos + 1);//从separator_pos+1位置开始，一直到行尾作为item_value
				trim(item_name);
				trim(item_value);
			}//例如line=" sex=girl",空格类型，那么separator_pos=3,item_name= sex，item_value=girl，trim后item_name变为sex
			else {
				item_name = line;//没找到一整行都是项目名称，这里不用trim，因为上面已经trim过了
				item_value = "";//项目值为空
			}

			item_node* new_item = new(nothrow) item_node;
			if (new_item == nullptr) {
				release_memory();
				return;
			}
			new_item->item_name = item_name;
			new_item->item_value = item_value;
			new_item->next = nullptr;
			new_item->original_line = original_item_line;

			if (current_group->items == nullptr) 
				current_group->items = new_item;//该组第一个项则项目指向new_item
			else {
				item_node* item_tail = current_group->items;
				while (item_tail->next != nullptr) {
					item_tail = item_tail->next;
				}//指向当前最后一个项
				item_tail->next = new_item;//加在最后一个项后面
			}
		}
	}
	in.close();//关文件
}


/***************************************************************************
  函数名称：
  功    能：构造函数初始化；调用解析配置文件函数
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
config_file_tools::config_file_tools(const char* const cfgname, const enum BREAK_CTYPE bctype) :
	cfgname(cfgname),
	item_separate_character_type(bctype),
	cfg_list(nullptr)
{
	parse_file();
}

/***************************************************************************
  函数名称：
  功    能：构造函数初始化；调用解析配置文件函数
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
config_file_tools::config_file_tools(const string& cfgname, const enum BREAK_CTYPE bctype) :
	cfgname(cfgname),
	item_separate_character_type(bctype),
	cfg_list(nullptr)
{
	parse_file();
}

/***************************************************************************
  函数名称：
  功    能：释放内存
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
config_file_tools::~config_file_tools()
{
	release_memory();
}

/***************************************************************************
  函数名称：
  功    能：判断读配置文件是否成功
  输入参数：
  返 回 值：true - 成功，已读入所有的组/项
		   false - 失败，文件某行超长/文件全部是注释语句
  说    明：如果cfg_list不为空，说明：1.parse里in文件打开成功 2.文件内容合法且有效，建立了链表
            此函数不应该负责解析，因为使用该类时未必调用这个函数
***************************************************************************/
bool config_file_tools::is_read_succeeded() const
{
	return cfg_list != nullptr;
}

/***************************************************************************
  函数名称：
  功    能：返回配置文件中的所有组
  输入参数：vector <string>& ret : vector 中每项为一个组名
  返 回 值：读到的组的数量（简单配置文件的组数量为1，组名为"）
  说    明：
***************************************************************************/
int config_file_tools::get_all_group(vector <string>& ret)
{
	ret.clear(); //先清空
	group_node* current = cfg_list;// 从链表头开始遍历
	bool simple_items = false;//simple_items表示是不是有内容的简单组（没有[]组名）
	while (current != nullptr) {//循环遍历所有由parse_file创建的组
		if (current->group_name.empty()) {//是不是简单组
			if (current->items != nullptr) 
				simple_items = true;//有配置项，说明是有内容的简单组，注意，此时不把 "" 加入 vector			
		}	
		else //正常配置文件		
			ret.push_back("[" + current->group_name + "]");// push_back函数作用：添加内容，把组名两侧加上[]后加入vector	
		current = current->next;
	}
	if (simple_items) // 循环结束后检查简单组，insert函数作用：把第一个参数位置的内容替换为第二个参数内容，空组组名为""
		ret.insert(ret.begin(), "");//begin()函数表示vector的第一个位置

	return ret.size();//size()函数表示vector的大小，即组的数量
}

/***************************************************************************
  函数名称：
  功    能：查找指定组的所有项并返回项的原始内容
  输入参数：const char* const group_name：组名
		   vector <string>& ret：vector 中每项为一个项的原始内容
		   const bool is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：项的数量，0表示空
  说    明：
***************************************************************************/
int config_file_tools::get_all_item(const char* const group_name, vector <string>& ret, const bool is_case_sensitive)
{
	ret.clear();
	string g_name = group_name;
	if (g_name.length() > 2 && g_name.front() == '[' && g_name.back() == ']') 
		g_name = g_name.substr(1, g_name.length() - 2);//如果是 "[group]" 格式，则提取组名
	else if (g_name == "[]") 
		g_name = "";//空组名 "[]"为空

	group_node* group = find_group(g_name, is_case_sensitive);//查找组
	if (group) {//找到了
		item_node* item = group->items;//指向该组的第一个项
		while (item != nullptr) {
			ret.push_back(item->original_line);//push_back函数作用：添加内容，把该项的原始行内容加入vector，此处加入原始行内容以保留格式
			item = item->next;
		}
	}

	return ret.size();
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int config_file_tools::get_all_item(const string& group_name, vector <string>& ret, const bool is_case_sensitive)
{
	return this->get_all_item(group_name.c_str(), ret, is_case_sensitive);//string转C风格字符串调用上一个函数即可
}

/***************************************************************************
  函数名称：
  功    能：取某项的原始内容（=后的所有字符，string方式）
  输入参数：const char* const group_name
		   const char* const item_name
		   string &ret
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：
***************************************************************************/
int config_file_tools::item_get_raw(const char* const group_name, const char* const item_name, string& ret, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string g_name = group_name;
	if (g_name.length() > 2 && g_name.front() == '[' && g_name.back() == ']') 
		g_name = g_name.substr(1, g_name.length() - 2);	
	else if (g_name == "[]") 
		g_name = "";//g_name为组名
	
	group_node* group = find_group(g_name, group_is_case_sensitive);
	item_node* item = find_item(group, item_name, item_is_case_sensitive);

	if (item) {
		ret = item->item_value;//找到了就把item_value赋给ret（和上面get_all_item不同，这里只返回值部分且已经trim）
		return 1;
	}
	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int config_file_tools::item_get_raw(const string& group_name, const string& item_name, string& ret, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_raw(group_name.c_str(), item_name.c_str(), ret, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为char型
  输入参数：const char* const group_name               ：组名
		   const char* const item_name                ：项名
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：1 - 该项的项名存在
		   0 - 该项的项名不存在
  说    明：
***************************************************************************/
int config_file_tools::item_get_null(const char* const group_name, const char* const item_name, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string g_name = group_name;
	if (g_name.length() > 2 && g_name.front() == '[' && g_name.back() == ']') 
		g_name = g_name.substr(1, g_name.length() - 2);
	else if (g_name == "[]") 
		g_name = "";

	group_node* group = find_group(g_name, group_is_case_sensitive);
	item_node* item = find_item(group, item_name, item_is_case_sensitive);

	if (item)
		return 1;
	else
		return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_null(const string& group_name, const string& item_name, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_null(group_name.c_str(), item_name.c_str(), group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为char型
  输入参数：const char* const group_name               ：组名
		   const char* const item_name                ：项名
		   char& value                                ：读到的char的值（返回1时可信，返回0则不可信）
		   const char* const choice_set = nullptr     ：合法的char的集合（例如："YyNn"表示合法输入为Y/N且不分大小写，该参数有默认值nullptr，表示全部字符，即不检查）
		   const char def_value = DEFAULT_CHAR_VALUE  ：读不到/读到非法的情况下的默认值，该参数有默认值DEFAULT_CHAR_VALUE，分两种情况
															当值是   DEFAULT_CHAR_VALUE 时，返回0（值不可信）
															当值不是 DEFAULT_CHAR_VALUE 时，令value=def_value并返回1
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：1 - 取到正确值
			   未取到值/未取到正确值，设置了缺省值（包括设为缺省值）
		   0 - 未取到（只有为未指定默认值的情况下才会返回0）
  说    明：
***************************************************************************/
int config_file_tools::item_get_char(const char* const group_name, const char* const item_name, char& value,
						const char* const choice_set, const char def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {//拿出了该项
		istringstream iss(raw_value);
		char v;
		iss >> v;
		if (!iss.fail()) {
			if (choice_set != nullptr) {//有限定合法字符
				const char* p = choice_set;
				bool found = false;
				while (*p) {
					if (*p == v) {
						found = true;
						break;
					}
					p++;
				}
				if (found) {//遍历后找到了合法字符
					value = v;
					return 1;
				}
			}
			else {//任意字符都合法
				value = v;
				return 1;
			}
		}
	}

	if (def_value != DEFAULT_CHAR_VALUE) {//没找到，但有默认值
		value = def_value;
		return 1;
	}

	return 0;//没取到值且def_value也没有指定，返回0
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_char(const string& group_name, const string& item_name, char& value,
						const char* const choice_set, const char def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_char(group_name.c_str(), item_name.c_str(), value, choice_set, def_value, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为int型
  输入参数：const char* const group_name               ：组名
		   const char* const item_name                ：项名
		   int& value                                 ：读到的int的值（返回1时可信，返回0则不可信）
		   const int min_value = INT_MIN              : 期望数据范围的下限，默认为INT_MIN
		   const int max_value = INT_MAX              : 期望数据范围的上限，默认为INT_MAX
		   const int def_value = DEFAULT_INT_VALUE    ：读不到/读到非法的情况下的默认值，该参数有默认值 DEFAULT_INT_VALUE，分两种情况
															当值是   DEFAULT_INT_VALUE 时，返回0（值不可信）
															当值不是 DEFAULT_INT_VALUE 时，令value=def_value并返回1
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：逻辑和item_get_char类似
***************************************************************************/
int config_file_tools::item_get_int(const char* const group_name, const char* const item_name, int& value,
						const int min_value, const int max_value, const int def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {
		istringstream iss(raw_value);
		int v;
		iss >> v;
		if (!iss.fail()) { // 确保成功提取了一个值
			if (v >= min_value && v <= max_value) {
				value = v;
				return 1;
			}
		}
	}

	if (def_value != DEFAULT_INT_VALUE) {
		value = def_value;
		return 1;
	}

	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_int(const string& group_name, const string& item_name, int& value,
						const int min_value, const int max_value, const int def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_int(group_name.c_str(), item_name.c_str(), value, min_value, max_value, def_value, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为double型
  输入参数：const char* const group_name                  ：组名
		   const char* const item_name                   ：项名
		   double& value                                 ：读到的int的值（返回1时可信，返回0则不可信）
		   const double min_value = __DBL_MIN__          : 期望数据范围的下限，默认为INT_MIN
		   const double max_value = __DBL_MAX__          : 期望数据范围的上限，默认为INT_MAX
		   const double def_value = DEFAULT_DOUBLE_VALUE ：读不到/读到非法的情况下的默认值，该参数有默认值DEFAULT_DOUBLE_VALUE，分两种情况
																当值是   DEFAULT_DOUBLE_VALUE 时，返回0（值不可信）
																当值不是 DEFAULT_DOUBLE_VALUE 时，令value=def_value并返回1
		   const bool group_is_case_sensitive = false     : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false      : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：
***************************************************************************/
int config_file_tools::item_get_double(const char* const group_name, const char* const item_name, double& value,
						const double min_value, const double max_value, const double def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {
		istringstream iss(raw_value);
		double v;
		iss >> v;
		if (!iss.fail()) { // 确保成功提取了一个值
			if (v >= min_value && v <= max_value) {
				value = v;
				return 1;
			}
		}
	}

	if (def_value != DEFAULT_DOUBLE_VALUE) {
		value = def_value;
		return 1;
	}
	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_double(const string& group_name, const string& item_name, double& value,
						const double min_value, const double max_value, const double def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_double(group_name.c_str(), item_name.c_str(), value, min_value, max_value, def_value, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为char * / char []型
  输入参数：const char* const group_name                  ：组名
		   const char* const item_name                   ：项名
		   char *const value                             ：读到的C方式的字符串（返回1时可信，返回0则不可信）
		   const int str_maxlen                          ：指定要读的最大长度（含尾零）
																如果<1则返回空串(不是DEFAULT_CSTRING_VALUE，虽然现在两者相同，但要考虑default值可能会改)
																如果>MAX_STRLEN 则上限为MAX_STRLEN
		   const char* const def_str                     ：读不到情况下的默认值，该参数有默认值DEFAULT_CSTRING_VALUE，分两种情况
																当值是   DEFAULT_CSTRING_VALUE 时，返回0（值不可信）
																当值不是 DEFAULT_CSTRING_VALUE 时，令value=def_value并返回1（注意，不是直接=）
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：1、为简化，未对\"等做转义处理，均按普通字符
		   2、含尾零的最大长度为str_maxlen，调用时要保证有足够空间
		   3、如果 str_maxlen 超过了系统预设的上限 MAX_STRLEN，则按 MAX_STRLEN 取
***************************************************************************/
int config_file_tools::item_get_cstring(const char* const group_name, const char* const item_name, char* const value,
						const int str_maxlen, const char* const def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (str_maxlen < 1) {//长度小于1，返回空串
		if (value != nullptr) 
			value[0] = '\0';
		return 0;
	}
	int max_len = (str_maxlen > MAX_STRLEN) ? MAX_STRLEN : str_maxlen;//如果str_maxlen超过了系统预设的上限MAX_STRLEN，则按MAX_STRLEN取

	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {
		istringstream iss(raw_value);
		string first_word;
		iss >> first_word; // 提取第一个单词
		if (!iss.fail()) {
			int len = (int)max_len - 1 > (int)first_word.length() ? (int)first_word.length() : (int)max_len - 1;
			strncpy(value, first_word.c_str(), len);// 复制第一个单词长度和max_len-1个字符较小的值
			value[len] = '\0'; // 添加字符串结束符
			return 1;
		}
	}

	if (def_value != DEFAULT_CSTRING_VALUE) {
		int len = (int)max_len - 1 > (int)strlen(def_value) ? (int)strlen(def_value) : (int)max_len - 1;
		strncpy(value, def_value, len);	// 复制默认值长度和max_len-1个字符较小的值			
		value[len] = '\0';
		return 1;
	}

	if (value != nullptr) 
		value[0] = '\0';//返回空串

	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_cstring(const string& group_name, const string& item_name, char* const value,
						const int str_maxlen, const char* const def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)

{
	/* 本函数已实现 */
	return item_get_cstring(group_name.c_str(), item_name.c_str(), value, str_maxlen, def_value, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为 string 型
  输入参数：const char* const group_name               ：组名
		   const char* const item_name                ：项名
		   string &value                              ：读到的string方式的字符串（返回1时可信，返回0则不可信）
		   const string &def_value                    ：读不到情况下的默认值，该参数有默认值DEFAULT_STRING_VALUE，分两种情况
															当值是   DEFAULT_STRING_VALUE 时，返回0（值不可信）
															当值不是 DEFAULT_STRING_VALUE 时，令value=def_value并返回1
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：为简化，未对\"等做转义处理，均按普通字符
***************************************************************************/
int config_file_tools::item_get_string(const char* const group_name, const char* const item_name, string& value,
						const string& def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {
		istringstream iss(raw_value);
		string first_word;
		iss >> first_word;
		if (!iss.fail()) {
			value = first_word;
			return 1;
		}
	}

	if (def_value != DEFAULT_STRING_VALUE) {
		value = def_value;
		return 1;
	}
	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_string(const string& group_name, const string& item_name, string& value,
						const string& def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_string(group_name.c_str(), item_name.c_str(), value, def_value, group_is_case_sensitive, item_is_case_sensitive);
}

/***************************************************************************
  函数名称：
  功    能：取某项的内容，返回类型为 IPv4 地址的32bit整型（主机序）
  输入参数：const char* const group_name               ：组名
		   const char* const item_name                ：项名
		   unsigned int &value                        ：读到的IP地址，32位整型方式（返回1时可信，返回0则不可信）
		   const unsigned int &def_value              ：读不到情况下的默认值，该参数有默认值DEFAULT_IPADDR_VALUE，分两种情况
															当值是   DEFAULT_IPADDR_VALUE 时，返回0（值不可信）
															当值不是 DEFAULT_IPADDR_VALUE 时，令value=def_value并返回1
		   const bool group_is_case_sensitive = false : 组名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
		   const bool item_is_case_sensitive = false  : 项名是否大小写敏感，true-大小写敏感 / 默认false-大小写不敏感
  返 回 值：
  说    明：
***************************************************************************/
int config_file_tools::item_get_ipaddr(const char* const group_name, const char* const item_name, unsigned int& value,
						const unsigned int& def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	string raw_value;
	if (item_get_raw(group_name, item_name, raw_value, group_is_case_sensitive, item_is_case_sensitive)) {
		istringstream iss(raw_value);
		int p1 = -1, p2 = -1, p3 = -1, p4 = -1;//初始化为-1，表示未赋值
		char dot1, dot2, dot3;

		iss >> p1 >> dot1 >> p2 >> dot2 >> p3 >> dot3 >> p4;

		if (!iss.fail() && dot1 == '.' && dot2 == '.' && dot3 == '.' &&
			p1 >= 0 && p1 <= 255 && p2 >= 0 && p2 <= 255 &&
			p3 >= 0 && p3 <= 255 && p4 >= 0 && p4 <= 255){
			value = (static_cast<unsigned int>(p1) << 24) |(static_cast<unsigned int>(p2) << 16) |
				(static_cast<unsigned int>(p3) << 8) | static_cast<unsigned int>(p4);//合法清空把四个部分合成一个32位整数
			return 1;
		}
	}

	if (def_value != DEFAULT_IPADDR_VALUE) {
		value = def_value;
		return 1;
	}

	return 0;
}

/***************************************************************************
  函数名称：
  功    能：组名/项目为string方式，其余同上
  输入参数：
  返 回 值：
  说    明：因为工具函数一般在程序初始化阶段被调用，不会在程序执行中被高频次调用，
		   因此这里直接套壳，会略微影响效率，但不影响整体性能（对高频次调用，此方法不适合）
***************************************************************************/
int config_file_tools::item_get_ipaddr(const string& group_name, const string& item_name, unsigned int& value,
						const unsigned int& def_value, const bool group_is_case_sensitive, const bool item_is_case_sensitive)
{
	/* 本函数已实现 */
	return this->item_get_ipaddr(group_name.c_str(), item_name.c_str(), value, def_value, group_is_case_sensitive, item_is_case_sensitive);
}
