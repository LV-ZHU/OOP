/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <ctime>
#include "../include/line_process_tools.h"
#include "../include/class_aat.h"
#include "../include/class_cft.h"
#include "../include/class_exe_runner.h"
#include "../include/class_tc.h"
#include "../include_mariadb_x86/mysql/mysql.h"
using namespace std;

typedef args_analyse_tools st_args_analyse;

struct stu_info
{
	string cno;//课号
	string stuno;//学号
	string name;//姓名
};

/***************************************************************************
  函数名称：get_basename_from_path
  功    能：从完整路径中提取文件名，和usage里的strchrr类似
  输入参数：p - 完整路径
  返 回 值：文件名部分
  说    明：支持 Windows/Linux 
***************************************************************************/
static string get_basename_from_path(const string& p)
{
	size_t pos = p.find_last_of("\\/");
	if (pos == string::npos)
		return p;
	return p.substr(pos + 1);
}

/***************************************************************************
  函数名称：excel_escape_stuno
  功    能：避免 Excel将13位号识别为科学计数法
  输入参数：stuno - 学号字符串
  返 回 值：转义后的字符串
  说    明：13位数字学号用 =text("xxx","#") 包装
***************************************************************************/
static string excel_escape_stuno(const string& stuno)
{
	if (stuno.size() == 13 && is_digits(stuno)) {
		ostringstream oss;
		oss << "=text(\"" << stuno << "\", \"#\")";
		return oss.str();
	}
	return stuno;
}

/***************************************************************************
  函数名称：strip_cfg_comment
  功    能：去除配置项字符串中的行内注释（// 和 #；适用于 raw 读取后表项）
  输入参数：s - 待处理字符串
  返 回 值：无
  说    明：- 先处理 //，再处理 #；并非同时去除
***************************************************************************/
static void strip_cfg_comment(string& s)
{
	size_t p = s.find("//");
	if (p != string::npos)
		s.erase(p);
	p = s.find_first_of(";#");
	if (p != string::npos) 
		s.erase(p);
	trim(s);  
}

/***************************************************************************
  函数名称：now_timestr_for_filename
  功    能：获取当前时间字符串（用于文件名，格式：YYYY-MM-DD-HH-MM-SS）
  返 回 值：格式化的时间字符串
 ***************************************************************************/
static string now_timestr_for_filename()
{
	time_t t = time(NULL);
	tm lt;
	localtime_s(&lt, &t);
	ostringstream oss;
	oss << (lt.tm_year + 1900) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_mon + 1) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_mday) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_hour) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_min) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_sec);
	return oss.str();
}

/***************************************************************************
  函数名称：now_timestr_for_screen
  功    能：获取当前时间字符串（用于屏幕输出，格式：YYYY-MM-DD HH:MM:SS）
  返 回 值：格式化的时间字符串，和上面的区别在于用：分隔日期和时间
***************************************************************************/
static string now_timestr_for_screen()
{
	time_t t = time(NULL);
	tm lt;
	localtime_s(&lt, &t);
	ostringstream oss;
	oss << (lt.tm_year + 1900) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_mon + 1) << "-";
	oss << setw(2) << setfill('0') << (lt.tm_mday) << " ";
	oss << setw(2) << setfill('0') << (lt.tm_hour) << ":";
	oss << setw(2) << setfill('0') << (lt.tm_min) << ":";
	oss << setw(2) << setfill('0') << (lt.tm_sec);
	return oss.str();
}

/***************************************************************************
  函数名称：file_exists_by_stat
  功    能：判断文件是否存在（使用 stat）
  输入参数：filepath - 文件路径
  返 回 值：true - 存在；false - 不存在
***************************************************************************/
static bool file_exists_by_stat(const string& filepath)
{
	struct stat st;
	return stat(filepath.c_str(), &st) == 0;
}

/***************************************************************************
  函数名称：ensure_dir_endslash
  功    能：将目录字符串补齐末尾分隔符
  输入参数：path - 输入/输出目录字符串
  返 回 值：无
  说    明：- 若 path 为空，不处理，若末尾不是 '\\' 或 '/'，则追加 '\\'
***************************************************************************/
static void ensure_dir_endslash(string& path)
{
	if (path.empty())
		return;
	char last = path[path.size() - 1];
	if (last != '\\' && last != '/')
		path.push_back('\\');
}

/***************************************************************************
  函数名称：trim_slashes_both_ends
  功    能：去除字符串首尾的斜杠
  输入参数：s - 输入/输出字符串
  返 回 值：无
  说    明：用于路径规范化
***************************************************************************/
static void trim_slashes_both_ends(string& s)
{
	while (!s.empty() && (s.front() == '\\' || s.front() == '/'))
		s.erase(s.begin());
	while (!s.empty() && (s.back() == '\\' || s.back() == '/'))
		s.pop_back();
}

/***************************************************************************
  函数名称：dir_exists_by_stat
  功    能：判断目录是否存在且可访问（stat 成功且为目录）
  输入参数：dir_with_or_without_slash - 目录路径（可带/不带尾分隔符）
  返 回 值：true  - 目录存在且可访问，false - 不存在或不可访问
  说    明："C:\" 这种盘符根目录保留（长度<=3时不再裁剪）
***************************************************************************/
static bool dir_exists_by_stat(const string& dir_with_or_without_slash)
{
	string dir = dir_with_or_without_slash;	
	while (dir.size() > 3) {/* 去掉尾部分隔符 */
		char c = dir[dir.size() - 1];
		if (c == '\\' || c == '/')
			dir.erase(dir.size() - 1);
		else
			break;
	}
	struct stat st;
	if (stat(dir.c_str(), &st) != 0)
		return false;
	if ((st.st_mode & _S_IFDIR) != 0)
		return true;
	return false;
}


/***************************************************************************
	函数名称：normalize_group_name
	功    能：将组名标准化为 "[xxx]" 形式
	输入参数：g - 输入/输出组名
	返 回 值：无
	说    明：若 g 为 "3-b3" 形式，转为 "[3-b3]"
 ***************************************************************************/
static void normalize_group_name(string& g)
{
	if (g.empty())
		return;
	if (!(g.front() == '[' && g.back() == ']'))
		g = "[" + g + "]";
}

/***************************************************************************
  函数名称：read_include
  功    能：读取组中的 include 配置项，并标准化为 "[xxx]" 形式
  输入参数：cfg- 配置文件对象，group_name - 当前组名，include_group - 输出的include组名
  返 回 值：true  - 存在且非空，false - 不存在或为空
  说    明：include 允许不存在或为空，返回值 include_group 已做 trim 与 normalize_group_name，
***************************************************************************/
static bool read_include(config_file_tools& cfg,
	const string& group_name,
	string& include_group)
{
	include_group.clear();
	string raw;
	if (cfg.item_get_raw(group_name, "include", raw) <= 0)
		return false;
	trim(raw);  
	if (raw.empty())
		return false;
	include_group = raw;
	normalize_group_name(include_group);
	return true;
}

/***************************************************************************
  函数名称：collect_include_chain_recursive
  功    能：递归收集include链（祖先在前）
  输入参数：cfg- 配置文件对象，group_name - 当前组名，chain_out -输出：include链
  返 回 值：无
  说    明：输出顺序：最先 include 的祖先在前，最近 include 的父组在后
***************************************************************************/
static void collect_include_chain_recursive(config_file_tools& cfg,
	const string& group_name,
	vector<string>& chain_out)
{
	string inc;
	if (read_include(cfg, group_name, inc)) {	
		collect_include_chain_recursive(cfg, inc, chain_out);/* 先递归到更上层（保持祖先在前的输出顺序） */
		chain_out.push_back(inc);	/* 再把本层 include 加入链 */
	}
}

/***************************************************************************
  函数名称：item_exists_including_include_chain
  功    能：判断配置项是否存在（包括 include 链）
  输入参数：cfg-配置文件对象，g-组名，key-配置项名
  返 回 值：true-存在；false-不存在
***************************************************************************/
static bool item_exists_including_include_chain(config_file_tools& cfg, const string& g, const string& key)
{
	if (cfg.item_get_null(g, key) > 0)
		return true;
	vector<string> chain;
	collect_include_chain_recursive(cfg, g, chain);
	for (int i = (int)chain.size() - 1; i >= 0; --i) {
		if (cfg.item_get_null(chain[i], key) > 0)
			return true;
	}
	return false;
}

/***************************************************************************
  函数名称：get_item_int_from_cfg
  功    能：从配置文件读取整型值（支持 include 链）
  输入参数：cfg-配置文件对象，g-组名，key-配置项名，out-输出的整型值
  返 回 值：true - 成功；false - 失败
***************************************************************************/
static bool get_item_int_from_cfg(config_file_tools& cfg, const string& g, const string& key, int& out)
{
	if (cfg.item_get_int(g, key, out) > 0)
		return true;
	vector<string> chain;
	collect_include_chain_recursive(cfg, g, chain);
	for (int i = (int)chain.size() - 1; i >= 0; --i) {
		if (cfg.item_get_int(chain[i], key, out) > 0)
			return true;
	}
	return false;
}

/***************************************************************************
  函数名称：get_item_string_from_cfg
  功    能：从配置文件中读取字符串配置项（优先 string，其次 raw），先下后上
  输入参数：sCfg-配置文件对象，g-组名，key-配置项名，out-输出字符串
  返 回 值：1-成功读到非空字符串，0-未读到（不存在或为空）
  说明：优先 item_get_string，其次 item_get_raw
***************************************************************************/
static int get_item_string_from_cfg(config_file_tools& sCfg, const string& g, const string& key, string& out)
{
	string raw;
	if (sCfg.item_get_raw(g, key, raw) > 0 && !raw.empty()) {
		strip_cfg_comment(raw);
		if (!raw.empty()) {
			out = raw;
			return 1;
		}
	}
	string val;
	int ret = sCfg.item_get_string(g, key, val);
	if (ret > 0 && !val.empty()) {
		strip_cfg_comment(val);
		if (!val.empty()) {
			out = val;
			return 1;
		}
	}
	vector<string> chain;
	collect_include_chain_recursive(sCfg, g, chain);
	for (int i = (int)chain.size() - 1; i >= 0; --i) {
		const string& pg = chain[i];
		string r;
		if (sCfg.item_get_raw(pg, key, r) > 0 && !r.empty()) {
			strip_cfg_comment(r);
			if (!r.empty()) {
				out = r;
				return 1;
			}
		}
		string v;
		if (sCfg.item_get_string(pg, key, v) > 0 && !v.empty()) {
			strip_cfg_comment(v);
			if (!v.empty()) {
				out = v;
				return 1;
			}
		}
	}
	out.clear();
	return 0;
}

/***************************************************************************
  函数名称：cft_get_int_or_default_with_error
  功    能：读取整型配置项并实现"默认处理逻辑"，同时收集错误信息
  输入参数：cfg-配置文件对象，g-组名，k-配置项名，minv/maxv-合法范围，
	        def_value-默认值（当配置项不存在时）
	        lower_when_invalid- 非法值处理时采用的值（一般为下限）
	        out-输出值，errors- 错误信息输出容器vector
  返 回 值：true-配置项存在（无论合法/非法），false-配置项不存在
***************************************************************************/
static bool cft_get_int_or_default_with_error(config_file_tools& cfg, const string& g, const string& k, int minv, int maxv,
	int def_value, int lower_when_invalid, int& out, vector<string>& errors)
{
	out = def_value;
	if (cfg.item_get_null(g, k) <= 0)
		return false;/* 配置项不存在 */

	string raw;
	if (cfg.item_get_raw(g, k, raw) > 0) {
		trim(raw); 
		strip_cfg_comment(raw);
		if (raw.empty() || raw == "<EMPTY>" || raw == "\"<EMPTY>\"") {
			out = def_value;
			ostringstream oss;
			oss << "配置项[" << k << "]为空，已按默认值" << def_value << "处理";
			errors.push_back(oss.str());
			return true; /* 配置项存在，但为空按默认值处理 */
		}
	}
	int v = 0;
	if (cfg.item_get_int(g, k, v) <= 0) {/* 非法（比如非数字）则按默认值处理 */
		out = def_value;
		ostringstream oss;
		oss << "配置项[" << k << "]不合法，已按默认值" << def_value << "处理（合法范围[" << minv << ".." << maxv << "]）";
		errors.push_back(oss.str());
		return true;
	}
	if (v >= minv && v <= maxv) {/*读到int：范围检查*/
		out = v;
		return true;
	}
	out = lower_when_invalid;
	ostringstream oss;
	oss << "配置项[" << k << "]不合法，已按" << lower_when_invalid
		<< "处理（合法范围[" << minv << ".." << maxv << "]）";
	errors.push_back(oss.str());

	return true;
}

/***************************************************************************
  函数名称：stu_less_for_sort
  功    能：学生信息排序比较函数
  输入参数：a, b两个学生信息
  返 回 值：true a < b；false a >= b
  说    明：学号升序；课号升序；再按姓名
***************************************************************************/
static bool stu_less_for_sort(const stu_info& a, const stu_info& b)
{
	if (a.stuno != b.stuno) 
		return a.stuno < b.stuno;
	if (a.cno != b.cno) 
		return a.cno < b.cno;
	return 
		a.name < b.name;
}

/***************************************************************************
  函数名称：sort_students_by_stuno
  功    能：按学号对学生列表排序
  输入参数：v - 学生信息列表
  返 回 值：无
  说    明：排序
***************************************************************************/
static void sort_students_by_stuno(vector<stu_info>& v)
{
	for (size_t i = 1; i < v.size(); ++i) {
		stu_info key = v[i];
		size_t j = i;
		while (j > 0 && stu_less_for_sort(key, v[j - 1])) {
			v[j] = v[j - 1];
			--j;
		}
		v[j] = key;
	}
}

/***************************************************************************
  函数名称：print_duplicate_students_if_any
  功    能：打印重复学生信息并从列表中删除，作业要求无但past-list有该情况
  输入参数：v-学生信息列表
  返 回 值：无
  说    明：排序后，相邻相同即重复（同一课号+学号视为重复），把重复项从 v 中删除，保证后续只输出一份
***************************************************************************/
static void print_duplicate_students_if_any(vector<stu_info>& v)
{
	if (v.empty())
		return;
	size_t write = 1;
	for (size_t read = 1; read < v.size(); ++read) {
		const stu_info& prev = v[write - 1];
		const stu_info& curr = v[read];
		if (prev.stuno == curr.stuno && prev.cno == curr.cno) {
			cout << now_timestr_for_screen() << " 课号=" << curr.cno << " 学号=" << curr.stuno << " 姓名=" << curr.name << " 重复." << endl;
			continue; 
		}
		if (write != read)
			v[write] = v[read];
		++write;
	}
	v.resize(write);
}

/***************************************************************************
  函数名称：parse_stulist_line
  功    能：解析学生名单文件的一行
  输入参数：line_raw - 原始行内容，out - 输出的学生信息
  返 回 值：true - 解析成功；false - 失败
  说    明：格式：课号 学号 姓名（空格分隔）
***************************************************************************/
static bool parse_stulist_line(const string& line_raw, stu_info& out)
{
	string line = line_raw;
	trim(line);  
	if (line.empty())
		return false;
	if (line[0] == '#')
		return false;
	istringstream iss(line);
	if (!(iss >> out.cno >> out.stuno >> out.name))
		return false;
	trim(out.cno);   
	trim(out.stuno); 
	trim(out.name);  
	if (!(out.cno.size() == 8 || out.cno.size() == 13) || !is_digits(out.cno))
		return false;
	if (out.stuno.size() != 7 || !is_digits(out.stuno))
		return false;
	if (out.name.empty())
		return false;
	return true;
}

/***************************************************************************
  函数名称：load_student_list_from_txt
  功    能：从文本文件加载学生名单
  输入参数：filename-文件名，out-输出的学生信息列表，errors-错误信息容器
  返 回 值：true成功；false失败
***************************************************************************/
static bool load_student_list_from_txt(const string& filename, vector<stu_info>& out, vector<string>& errors)
{
	out.clear();
	ifstream in(filename.c_str(), ios::in | ios::binary);
	if (!in.is_open()) {
		errors.push_back("学生名单文件无法打开: " + filename);
		return false;
	}
	string line;
	while (getline(in, line)) {
		if (!line.empty() && line.back() == '\r')/* Windows/Linux*/
			line.pop_back();
		stu_info si;
		if (!parse_stulist_line(line, si))
			continue;
		out.push_back(si);
	}
	return true;
}

/***************************************************************************
  函数名称：db_connect_and_query_students
  功    能：从数据库查询学生名单
  输入参数：cfg- 配置文件对象，out-输出的学生信息列表，errors-错误信息容器
  返 回 值：true-成功；false-失败
  说    明：从 [数据库] 组读取配置并查询
***************************************************************************/
static bool db_connect_and_query_students(config_file_tools& cfg, vector<stu_info>& out, vector<string>& errors)
{
	out.clear();
	string db_host, db_name, db_username, db_passwd, db_cno_list;
	get_item_string_from_cfg(cfg, "[数据库]", "db_host", db_host);
	get_item_string_from_cfg(cfg, "[数据库]", "db_name", db_name);
	get_item_string_from_cfg(cfg, "[数据库]", "db_username", db_username);
	get_item_string_from_cfg(cfg, "[数据库]", "db_passwd", db_passwd);
	get_item_string_from_cfg(cfg, "[数据库]", "db_cno_list", db_cno_list);
	trim(db_host);    
	trim(db_name);    
	trim(db_username); 
	trim(db_passwd);   
	trim(db_cno_list); 
	if (db_host.empty() || db_name.empty() || db_username.empty() || db_passwd.empty() || db_cno_list.empty()) {
		errors.push_back("[数据库]组配置不完整（db_host/db_name/db_username/db_passwd/db_cno_list）");
		return false;
	}
	vector<string> cnos;
	string s = db_cno_list;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == ',' || s[i] == ';' || s[i] == '\t')
			s[i] = ' ';
	}
	istringstream iss(s);
	string x;
	while (iss >> x) {
		trim(x);  // 使用 line_process_tools.h 提供的 trim 函数
		if (!x.empty())
			cnos.push_back(x);
	}
	if (cnos.empty()) {
		errors.push_back("[数据库]db_cno_list 为空或无法解析");
		return false;
	}

	MYSQL* mysql = mysql_init(NULL);
	if (mysql == NULL) {
		errors.push_back("mysql_init 失败");
		return false;
	}
	if (mysql_real_connect(mysql, db_host.c_str(), db_username.c_str(), db_passwd.c_str(), db_name.c_str(), 0, NULL, 0) == NULL) {
		errors.push_back(string("mysql_real_connect 失败: ") + mysql_error(mysql));
		mysql_close(mysql);
		return false;
	}
	mysql_set_character_set(mysql, "gbk");
	/* stu_cno in('xxx','yyy') */
	ostringstream in_clause;
	for (size_t i = 0; i < cnos.size(); ++i) {
		if (i) 
			in_clause << ",";
		in_clause << "\"" << cnos[i] << "\"";
	}
	string sql = "select stu_cno, stu_no, stu_name from view_student_for_oop where stu_cno in (" + in_clause.str() + ") and stu_enable!='0'";
	if (mysql_query(mysql, sql.c_str()) != 0) {
		errors.push_back(string("mysql_query 失败: ") + mysql_error(mysql));
		mysql_close(mysql);
		return false;
	}
	MYSQL_RES* result = mysql_store_result(mysql);
	if (result == NULL) {
		errors.push_back("mysql_store_result 失败");
		mysql_close(mysql);
		return false;
	}
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result)) != NULL) {
		stu_info si;
		si.cno = row[0] ? row[0] : "";
		si.stuno = row[1] ? row[1] : "";
		si.name = row[2] ? row[2] : "";
		trim(si.cno);
		trim(si.stuno);
		trim(si.name);  
		out.push_back(si);
	}
	mysql_free_result(result);
	mysql_close(mysql);
	return true;
}

/***************************************************************************
	函数名称：build_student_exe_fullpath
	功    能：根据 exe_style 构建学生 exe 完整路径
	输入参数：exe_style- exe_style 配置项，single_exe_dirname-single模式目录，multi_exe_main_dirname-multi模式主目录，
	          multi_exe_sub_dirname-multi 模式子目录，cno-课号，stuno-学号，stu_exe_name-exe 文件名，
	返 回 值：完整路径字符串
***************************************************************************/
static string build_student_exe_fullpath(const string& exe_style,const string& single_exe_dirname,const string& multi_exe_main_dirname,
	const string& multi_exe_sub_dirname,const string& cno,const string& stuno,const string& stu_exe_name)
{
	if (exe_style == "single") { /* single: 目录下文件名为 学号-课号-exe_name */
		ostringstream oss;
		oss << single_exe_dirname << stuno << "-" << cno << "-" << stu_exe_name;
		return oss.str();
	}
	if (exe_style == "multi") { /* multi: 根目录/学号-课号/子目录/exe名 */
		ostringstream oss;
		oss << multi_exe_main_dirname << stuno << "-" << cno << "\\" << multi_exe_sub_dirname << stu_exe_name;
		return oss.str();
	}
	return "";
}

/***************************************************************************
  函数名称：resolve_student_exe_fullpath
  功    能：返回第一个存在的 exe 路径，否则返回空串
  输入参数：tried-可选的输出参数，记录尝试过的路径
  返 回 值：第一个存在的路径，或空串
  说    明：multi模式增加路径尝试
***************************************************************************/
static string resolve_student_exe_fullpath(const string& exe_style,const string& single_exe_dirname,const string& multi_exe_main_dirname,
	const string& multi_exe_sub_dirname,const string& cno,const string& stuno,const string& stu_exe_name,vector<string>* tried)
{
	vector<string> local_tried;
	vector<string>& T = (tried ? *tried : local_tried);
	string p = build_student_exe_fullpath(exe_style, single_exe_dirname,
		multi_exe_main_dirname, multi_exe_sub_dirname, cno, stuno, stu_exe_name);
	if (!p.empty()) {
		T.push_back(p);
		if (file_exists_by_stat(p))
			return p;
	}
	if (exe_style == "multi") {
		ostringstream oss;
		oss << multi_exe_main_dirname << stuno << "-" << cno << "\\" << stu_exe_name;
		string p = oss.str();
		T.push_back(p);
		if (file_exists_by_stat(p))
			return p;
		string sub = multi_exe_sub_dirname;
		trim(sub);  // 使用 line_process_tools.h 提供的 trim 函数
		trim_slashes_both_ends(sub);

		if (!sub.empty()) {
			ostringstream oss;
			oss << multi_exe_main_dirname << stuno << "-" << cno << "\\" << sub << "\\" << stu_exe_name;
			string p = oss.str();
			T.push_back(p);
			if (file_exists_by_stat(p))
				return p;
		}
	}

	return "";
}

/***************************************************************************
  函数名称：build_demo_exec_cmd
  功    能：构建 demo 程序执行命令
  返 回 值：完整命令字符串
***************************************************************************/
static string build_demo_exec_cmd(const string& cmd_style,const string& demo_exe_name,const string& pipe_get_input_data_exe_name,
	const string& pipe_data_file,const string& redirection_data_dirname,const string& item_value)
{
	ostringstream cmd;
	if (cmd_style == "normal") {
		cmd << demo_exe_name;
		return cmd.str();
	}
	if (cmd_style == "pipe") {
		string g = item_value;
		trim(g);  
		if (!g.empty() && !(g.front() == '[' && g.back() == ']'))
			g = "[" + g + "]";
		cmd << (pipe_get_input_data_exe_name) << " " << (pipe_data_file) << " " << g << " | " << (demo_exe_name);
		return cmd.str();
	}
	if (cmd_style == "redirection") {
		cmd << (demo_exe_name) << " < " << (redirection_data_dirname + item_value);
		return cmd.str();
	}
	if (cmd_style == "main_with_arguments") {
		cmd << (demo_exe_name);
		if (!item_value.empty())
			cmd << " " << item_value;
		return cmd.str();
	}

	return "";
}

/***************************************************************************
  函数名称：build_student_exec_cmd
  功    能：构建学生程序执行命令
  返 回 值：完整命令字符串
***************************************************************************/
static string build_student_exec_cmd(const string& cmd_style,const string& student_exe_fullpath,const string& pipe_get_input_data_exe_name,
	const string& pipe_data_file,const string& redirection_data_dirname,const string& item_value)
{
	ostringstream cmd;
	if (cmd_style == "normal") {
		cmd << (student_exe_fullpath);
		return cmd.str();
	}
	if (cmd_style == "pipe") {
		string g = item_value;
		trim(g); 
		if (!g.empty() && !(g.front() == '[' && g.back() == ']'))
			g = "[" + g + "]";
		cmd << (pipe_get_input_data_exe_name) << " " << (pipe_data_file) << " " << g << " | " << (student_exe_fullpath);
		return cmd.str();
	}
	if (cmd_style == "redirection") {
		cmd << (student_exe_fullpath)
			<< " < "
			<< (redirection_data_dirname + item_value);
		return cmd.str();
	}
	if (cmd_style == "main_with_arguments") {
		cmd << (student_exe_fullpath);
		if (!item_value.empty())
			cmd << " " << item_value;
		return cmd.str();
	}

	return "";
}

/***************************************************************************
  函数名称：collect_items_values
  功    能：收集测试项配置值
  输入参数：cfg-配置文件对象，gname-组名，cmd_style-cmd_style 配置项，items_num-测试项数量，
			out_items-输出的测试项值列表，errors-错误信息容器vector
  返 回 值：无
  说    明： 修改：允许配置项为空（配置文件注释明确说明 item_args_32 = 允许为空）
***************************************************************************/
static void collect_items_values(config_file_tools& cfg, const string& gname, const string& cmd_style, int items_num,
	vector<string>& out_items, vector<string>& errors)
{
	out_items.clear();
	out_items.reserve((size_t)items_num);

	for (int i = 1; i <= items_num; ++i) {
		ostringstream key;
		if (cmd_style == "pipe")
			key << "item_gname_" << i;
		else if (cmd_style == "redirection")
			key << "item_fname_" << i;
		else if (cmd_style == "main_with_arguments")
			key << "item_args_" << i;
		else
			break;

		string v;
		/*  核心修改：不存在或为空时，用空字符串填充（而非报错返回） */
		if (get_item_string_from_cfg(cfg, gname, key.str(), v) <= 0) {
			v = "";  // 允许为空（配置文件注释中说明 item_args_32 = 允许为空）
		}

		trim(v);
		out_items.push_back(v);  //  保证 out_items.size() == items_num
	}
}

/***************************************************************************
  函数名称：calc_field_width
  功    能：计算输出对齐所需的 label 字段宽度
  输入参数：keys-要输出的 key 列表，items_num_for_label-是否考虑item_name_N的最大长度
  返 回 值：最大 label 长度
  说    明：仅用于对齐输出
***************************************************************************/
static int calc_field_width(const vector<pair<string, string> >& keys, int items_num_for_label)
{
	int max_label_len = 0;
	for (size_t i = 0; i < keys.size(); ++i) {
		int L = (int)keys[i].first.length();
		if (L > max_label_len)
			max_label_len = L;
	}
	for (int i = 1; i <= items_num_for_label; ++i) {
		ostringstream oss;
		oss << "item_name_" << i;
		int L = (int)oss.str().length();
		if (L > max_label_len)
			max_label_len = L;
	}
	return max_label_len;
}

/***************************************************************************
  函数名称：print_section_header
  功    能：打印配置段头（横线 + "xxx配置信息如下"）
  输入参数：group_name - 组名（"[xxx]"形式）
  返 回 值：无
  说    明：
***************************************************************************/
static void print_section_header(const string& group_name)
{
	cout << endl << string(100, '=') << endl;
	cout << (group_name.empty() ? "" : group_name) << "配置信息如下：" << endl;
	cout << string(100, '=') << endl;
}

/***************************************************************************
  函数名称：push_error_items_num_min
  功    能：追加 items_num 最小值错误信息
  输入参数：v-实际（可能非法）值，errors - 错误信息容器
  返 回 值：无
***************************************************************************/
static void push_error_items_num_min(int v, vector<string>& errors)
{
	ostringstream oss;
	oss << "items_num[" << v << "]最小为1";
	errors.push_back(oss.str());
}

/***************************************************************************
  函数名称：push_error_items_end_gt_num
  功    能：追加 items_end > items_num 的错误信息
  输入参数：endv-items_end 值，numv-items_num 值，errors-错误信息容器
  返 回 值：无
***************************************************************************/
static void push_error_items_end_gt_num(int endv, int numv, vector<string>& errors)
{
	ostringstream oss;
	oss << "items_end[" << endv << "] > items_num[" << numv << "]";
	errors.push_back(oss.str());
}

/***************************************************************************
  函数名称：items_pack_complete
  功    能：判断一个组内 items 配置是否"完整且合法"
  输入参数：cfg-配置文件对象，gname-组名，cmd_style-当前组的 cmd_style，
	        out_items_num-输出，若完整则返回合法items_num，否则为0
  返 回 值：true-items_num 与对应item_*同时存在，且items_num 可读为合法整数，
	        false-不完整或非法
  说    明：用于决定是否打印 item_name_1..N注意这里只检查 item_*_1 的存在性，目的是是否应打印列表，不是校验每一项是否存在
***************************************************************************/
static bool items_pack_complete(config_file_tools& cfg, const string& gname, const string& cmd_style, int& out_items_num)
{
	out_items_num = 0;
	string k1;
	if (cmd_style == "pipe")
		k1 = "item_gname_1";
	else if (cmd_style == "redirection")
		k1 = "item_fname_1";
	else if (cmd_style == "main_with_arguments")
		k1 = "item_args_1";
	else
		return false;
	/* 必须同时"存在"：允许存在于 include 链 */
	if (!item_exists_including_include_chain(cfg, gname, "items_num"))
		return false;
	if (!item_exists_including_include_chain(cfg, gname, k1))
		return false;
	/* items_num 必须能读成合法整数（>=1）：允许从 include 链读取 */
	int n = 0;
	if (!get_item_int_from_cfg(cfg, gname, "items_num", n))
		return false;
	if (n < 1)
		return false;

	out_items_num = n;
	return true;
}


/***************************************************************************
  函数名称：print_group_cfg_checkonly
  功    能：按 demo 风格打印某个组的配置信息（仅 checkcfg_only 使用）
  输入参数：cfg, group_name - 配置对象/组名，exe_style/name_list/cmd_style-三个变化要素，items_num/items_begin/items_end-测试项数相关
	timeout_sec/max_output_len - 超时与输出长度限制，tc_* - 文本比较参数，*_dirname - 若涉及目录项，传入已修正为带尾分隔符的字符串
	print_item_names-是否打印 item_name_1..N
  返 回 值：无
  说    明：
***************************************************************************/
static void print_group_cfg_checkonly(config_file_tools& cfg,const string& group_name,const string& exe_style,const string& name_list,const string& cmd_style,
	int items_num,int items_begin,int items_end,int timeout_sec,int max_output_len,int tc_lineskip,int tc_lineoffset,int tc_ignoreblank,int tc_not_ignore_linefeed,
	int tc_maxdiff,int tc_maxline,const string& single_exe_dirname,const string& multi_exe_main_dirname,const string& multi_exe_sub_dirname,const string& redirection_data_dirname,bool print_item_names)
{
	cout << (group_name.empty() ? "" : group_name) << "：" << endl;
	vector<pair<string, string> > keys;
	keys.push_back(make_pair(string("exe_style"), string("exe_style")));
	/* exe_style != none 才打印 name_list*/
	if (exe_style != "none")
		keys.push_back(make_pair(string("name_list"), string("name_list")));
	/* exe_style 分支 */
	if (exe_style == "single") {
		keys.push_back(make_pair(string("single_exe_dirname"), string("single_exe_dirname")));
		keys.push_back(make_pair(string("stu_exe_name"), string("stu_exe_name")));
	}
	else if (exe_style == "multi") {
		keys.push_back(make_pair(string("multi_exe_main_dirname"), string("multi_exe_main_dirname")));
		keys.push_back(make_pair(string("multi_exe_sub_dirname"), string("multi_exe_sub_dirname")));
		keys.push_back(make_pair(string("stu_exe_name"), string("stu_exe_name")));
	}
	/* demo_exe_name：raw 打印 */
	keys.push_back(make_pair(string("demo_exe_name"), string("demo_exe_name")));
	keys.push_back(make_pair(string("cmd_style"), string("cmd_style")));
	keys.push_back(make_pair(string("max_output_len"), string("max_output_len")));
	keys.push_back(make_pair(string("timeout"), string("timeout")));
	/* cmd_style 分支 */
	if (cmd_style == "pipe") {
		keys.push_back(make_pair(string("pipe_get_input_data_exe_name"), string("pipe_get_input_data_exe_name")));
		keys.push_back(make_pair(string("pipe_data_file"), string("pipe_data_file")));
	}
	else if (cmd_style == "redirection") {
		keys.push_back(make_pair(string("redirection_data_dirname"), string("redirection_data_dirname")));
	}
	/* tc_* */
	keys.push_back(make_pair(string(""), string("")));
	keys.push_back(make_pair(string("tc_trim"), string("tc_trim")));
	keys.push_back(make_pair(string("tc_lineskip"), string("tc_lineskip")));
	keys.push_back(make_pair(string("tc_lineoffset"), string("tc_lineoffset")));
	keys.push_back(make_pair(string("tc_ignoreblank"), string("tc_ignoreblank")));
	keys.push_back(make_pair(string("tc_not_ignore_linefeed"), string("tc_not_ignore_linefeed")));
	keys.push_back(make_pair(string("tc_maxdiff"), string("tc_maxdiff")));
	keys.push_back(make_pair(string("tc_maxline"), string("tc_maxline")));
	keys.push_back(make_pair(string("tc_display"), string("tc_display")));
	/* items_* */
	keys.push_back(make_pair(string(""), string("")));
	keys.push_back(make_pair(string("items_num"), string("items_num")));
	keys.push_back(make_pair(string("items_begin"), string("items_begin")));
	keys.push_back(make_pair(string("items_end"), string("items_end")));
	/* 控制对齐宽度：是否考虑 item_name_N 的 label */
	int label_items_num = 0;
	if ((cmd_style == "pipe" || cmd_style == "redirection" || cmd_style == "main_with_arguments") && print_item_names)
		label_items_num = items_num;
	int field_width = calc_field_width(keys, label_items_num);
	/* 逐项输出 */
	for (size_t i = 0; i < keys.size(); ++i) {
		const string& label = keys[i].first;
		const string& key = keys[i].second;
		if (label.empty()) {
			cout << endl;
			continue;
		}
		cout << "  " << setiosflags(ios::left) << setw(field_width) << label << " = ";
		/* 目录项：打印外部传入值（已做 trim + 补斜杠） */
		if (key == "single_exe_dirname") {
			cout << single_exe_dirname << endl;
			continue;
		}
		if (key == "multi_exe_main_dirname") {
			cout << multi_exe_main_dirname << endl;
			continue;
		}
		if (key == "multi_exe_sub_dirname") {
			cout << multi_exe_sub_dirname << endl;
			continue;
		}
		if (key == "redirection_data_dirname") {
			cout << redirection_data_dirname << endl;
			continue;
		}
		/* demo_exe_name：raw 打印（允许带参数） */
		if (key == "demo_exe_name") {
			string raw;
			get_item_string_from_cfg(cfg, group_name, key, raw);
			cout << raw << endl;
			continue;
		}
		else if (key == "items_num") {
			cout << items_num << endl;
			continue;
		}
		else if (key == "items_begin") {
			cout << items_begin << endl;
			continue;
		}
		else if (key == "items_end") {
			cout << items_end << endl;
			continue;
		}
		/* timeout/max_output_len：打印已解析后的整数 */
		if (key == "timeout") {
			cout << timeout_sec << endl;
			continue;
		}
		if (key == "max_output_len") {
			cout << max_output_len << endl;
			continue;
		}
		/* tc_*：打印已解析后的整数 */
		if (key == "tc_lineskip") {
			cout << tc_lineskip << endl;
			continue;
		}
		if (key == "tc_lineoffset") {
			cout << tc_lineoffset << endl;
			continue;
		}
		if (key == "tc_ignoreblank") {
			cout << tc_ignoreblank << endl;
			continue;
		}
		if (key == "tc_not_ignore_linefeed") {
			cout << tc_not_ignore_linefeed << endl;
			continue;
		}
		if (key == "tc_maxdiff") {
			cout << tc_maxdiff << endl;
			continue;
		}
		if (key == "tc_maxline") {
			cout << tc_maxline << endl;
			continue;
		}
		/* 其余项按include优先级读取 */
		string val;
		get_item_string_from_cfg(cfg, group_name, key, val);
		cout << val << endl;
	}
	/* item_name_N（仅 pipe/redirection/main_with_arguments 且 items 完整时打印） */
	if (print_item_names && (cmd_style == "pipe" || cmd_style == "redirection" || cmd_style == "main_with_arguments")) {
		for (int i = 1; i <= items_num; ++i) {
			ostringstream label;
			label << "item_name_" << i;

			ostringstream cfgkey;
			if (cmd_style == "pipe")
				cfgkey << "item_gname_" << i;
			else if (cmd_style == "redirection")
				cfgkey << "item_fname_" << i;
			else
				cfgkey << "item_args_" << i;
			cout << "  " << setiosflags(ios::left) << setw(field_width) << label.str() << " = ";
			string raw;
			if (get_item_string_from_cfg(cfg, group_name, cfgkey.str(), raw) > 0) {
				if (cmd_style == "pipe") {
					trim(raw);
					if (!raw.empty() && raw.front() == '[' && raw.back() == ']')
						cout << raw;
					else if (!raw.empty())
						cout << "[" << raw << "]";
				}
				else {
					cout << raw;
				}
			}
			cout << endl;
		}
	}
}

/***************************************************************************
  函数名称：checkcfg_only_print_one_group
  功    能：只打印一个组的配置信息
  输入参数：sCfg-配置文件对象，gname-组名，errors-错误信息容器
  返 回 值：无
  说    明：
***************************************************************************/
static void checkcfg_only_print_one_group(config_file_tools& sCfg,const string& gname,vector<string>& errors)
{
	string exe_style;
	string name_list;
	string cmd_style;
	get_item_string_from_cfg(sCfg, gname, "exe_style", exe_style);
	get_item_string_from_cfg(sCfg, gname, "name_list", name_list);
	get_item_string_from_cfg(sCfg, gname, "cmd_style", cmd_style);
	int items_num = 32;
	int raw_items_num = 0;
	bool items_num_exist = item_exists_including_include_chain(sCfg, gname, "items_num");
	if (items_num_exist) {
		if (get_item_int_from_cfg(sCfg, gname, "items_num", raw_items_num)) {
			if (raw_items_num < 1) {
				items_num = 0;
				push_error_items_num_min(raw_items_num, errors);
			}
			else if (raw_items_num > 1024) {
				items_num = 0;
			}
			else {
				items_num = raw_items_num;
			}
		}
		else {
			items_num = 0;
		}
	}
	int items_begin = 0;
	int items_end = 0;
	int tmp = 0;
	if (get_item_int_from_cfg(sCfg, gname, "items_begin", tmp) && tmp >= 1 && tmp <= 1024)
		items_begin = tmp;
	else
		items_begin = 0;
	if (get_item_int_from_cfg(sCfg, gname, "items_end", tmp) && tmp >= 1 && tmp <= 1024)
		items_end = tmp;
	else {
		if (items_num > 0)
			items_end = items_num;
		else
			items_end = 32;
	}
	if (items_end > items_num && items_num == 0)
		push_error_items_end_gt_num(items_end, items_num, errors);
	/* timeout/max_output_len */
	int timeout_sec = 0;
	int max_output_len = 0;
	cft_get_int_or_default_with_error(sCfg, gname, "timeout", 1, 3600, 3, 1, timeout_sec, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "max_output_len", 1, 65536, 256, 1, max_output_len, errors);
	/* tc_* */
	int tc_lineskip = 0;
	int tc_lineoffset = 0;
	int tc_ignoreblank = 0;
	int tc_not_ignore_linefeed = 0;
	int tc_maxdiff = 0;
	int tc_maxline = 0;
	cft_get_int_or_default_with_error(sCfg, gname, "tc_lineskip", 0, 100, 0, 0, tc_lineskip, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "tc_lineoffset", -100, 100, 0, -100, tc_lineoffset, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "tc_ignoreblank", 0, 1, 0, 0, tc_ignoreblank, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "tc_not_ignore_linefeed", 0, 1, 0, 0, tc_not_ignore_linefeed, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "tc_maxdiff", 0, 100, 0, 0, tc_maxdiff, errors);
	cft_get_int_or_default_with_error(sCfg, gname, "tc_maxline", 0, 10000, 0, 0, tc_maxline, errors);
	string single_exe_dirname;
	string multi_exe_main_dirname;
	string multi_exe_sub_dirname;
	string redirection_data_dirname;
	if (exe_style == "single") {
		get_item_string_from_cfg(sCfg, gname, "single_exe_dirname", single_exe_dirname);
		trim(single_exe_dirname);
		ensure_dir_endslash(single_exe_dirname);
		if (!single_exe_dirname.empty() && !dir_exists_by_stat(single_exe_dirname))
			errors.push_back("配置项[single_exe_dirname]指定目录不存在或不可访问");
	}
	else if (exe_style == "multi") {
		get_item_string_from_cfg(sCfg, gname, "multi_exe_main_dirname", multi_exe_main_dirname);
		trim(multi_exe_main_dirname);
		ensure_dir_endslash(multi_exe_main_dirname);
		if (!multi_exe_main_dirname.empty() && !dir_exists_by_stat(multi_exe_main_dirname))
			errors.push_back("配置项[multi_exe_main_dirname]指定目录不存在或不可访问");
		get_item_string_from_cfg(sCfg, gname, "multi_exe_sub_dirname", multi_exe_sub_dirname);
		trim(multi_exe_sub_dirname);
		ensure_dir_endslash(multi_exe_sub_dirname);
	}
	if (cmd_style == "redirection") {
		get_item_string_from_cfg(sCfg, gname, "redirection_data_dirname", redirection_data_dirname);
		trim(redirection_data_dirname);
		ensure_dir_endslash(redirection_data_dirname);
		if (!redirection_data_dirname.empty() && !dir_exists_by_stat(redirection_data_dirname))
			errors.push_back("配置项[redirection_data_dirname]指定目录不存在或不可访问");
	}
	/* [数据库]打印仅当 exe_style != none 且 name_list == database */
	if (exe_style != "none" && name_list == "database") {
		cout << "[数据库]：" << endl;
		string tmp_s;
		get_item_string_from_cfg(sCfg, "[数据库]", "db_host", tmp_s);
		cout << "  db_host      = " << tmp_s << endl;
		int db_port = 0;
		if (sCfg.item_get_int("[数据库]", "db_port", db_port) > 0)
			cout << "  db_port      = " << db_port << endl;
		else
			cout << "  db_port      = " << "" << endl;
		get_item_string_from_cfg(sCfg, "[数据库]", "db_name", tmp_s);
		cout << "  db_name      = " << tmp_s << endl;
		get_item_string_from_cfg(sCfg, "[数据库]", "db_username", tmp_s);
		cout << "  db_username  = " << tmp_s << endl;
		get_item_string_from_cfg(sCfg, "[数据库]", "db_curr_term", tmp_s);
		cout << "  db_curr_term = " << tmp_s << endl;
		get_item_string_from_cfg(sCfg, "[数据库]", "db_cno_list", tmp_s);
		cout << "  db_cno_list  = " << tmp_s << endl;
		cout << endl;
	}
	/* 决定是否打印 item_name_1..N：满足items_num 与 item_* 同时存在且 items_num 合法 */
	bool print_item_names = false;
	int n_ok = 0;
	if (items_pack_complete(sCfg, gname, cmd_style, n_ok))
		print_item_names = true;
	print_group_cfg_checkonly(sCfg,gname,exe_style,name_list,cmd_style,items_num,items_begin,items_end,timeout_sec,max_output_len,tc_lineskip,tc_lineoffset,
		tc_ignoreblank,tc_not_ignore_linefeed,tc_maxdiff,tc_maxline,single_exe_dirname,multi_exe_main_dirname,multi_exe_sub_dirname,redirection_data_dirname,print_item_names);
	cout << string(100, '=') << endl << endl;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
static void usage(const char* const full_procname)
{
	const int offset_len = 7;

	const char* procname = strrchr(full_procname, '\\');
	if (procname == NULL)
		procname = full_procname;

	cout << "Usage: " << procname << " --必选项 | --可选项(出现次序不限)" << endl << endl;

	cout << "必选项：指定操作" << endl;
	cout << "   --checkname : 需要检查的项目名(配置文件的组名)" << endl << endl;

	cout << "必选项[--checkname]的可选参数：" << endl;
	cout << "   --checkcfg_only : 仅打印配置文件检查结果" << endl << endl;

	cout << "可选项：指定操作(任选0~n个)" << endl;
	cout << "   --help    : 打印帮助信息" << endl;
	cout << "   --debug   : 设置Debug等级(可设等级 : warn/info/debug/trace，后项均包含前项，例：info必然包含warn信息)" << endl;
	cout << "                   fatal : 致命错误(不能设置，已打开)" << endl;
	cout << "                   error : 程序异常错误(不能设置，已打开)" << endl;
	cout << "                   warn  : 提示潜在问题，但不影响程序运行(默认项)" << endl;
	cout << "                   info  : 记录程序运行状态信息" << endl;
	cout << "                   debug : 打印程序调试过程中的详细信息(打开后会影响程序执行速度，仅建议Debug时使用)" << endl;
	cout << "                   trace : 比debug更详细的调试信息(打开后会影响程序执行速度，仅建议Debug时使用)" << endl;
	cout << "   --cfgfile : 指定配置文件" << endl << endl;

	cout << "e.g.  :" << endl;
	cout << setw(offset_len) << ' ' << procname << " --checkname 3-b3                 : 按配置文件[3-b3]组的设定检查exe的运行结果" << endl;
	cout << setw(offset_len) << ' ' << procname << " --checkname 3-b3 --checkcfg_only : 检查配置文件[3-b3]组的设定是否正确" << endl;
	cout << setw(offset_len) << ' ' << procname << " --checkname 3-b3 --debug trace   : 按配置文件[3-b3]组的设定检查exe的运行结果，打印所有调试信息" << endl;
	cout << endl;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int main(int argc, char** argv)
{
	const string loglevel_define[] = { "warn", "info", "debug", "trace", "" };

	st_args_analyse args[] = {
		st_args_analyse("--help",           ST_EXTARGS_TYPE::boolean,               0, false),
		st_args_analyse("--debug",          ST_EXTARGS_TYPE::str_with_set_default,  1, 0, loglevel_define),
		st_args_analyse("--checkname",      ST_EXTARGS_TYPE::str,                   1, string("")),
		st_args_analyse("--checkcfg_only",  ST_EXTARGS_TYPE::boolean,               0, false),
		st_args_analyse("--cfgfile",        ST_EXTARGS_TYPE::str,                   1, string("hw_check_exe.cfg")),
		st_args_analyse()
	};

	/* 解析命令行参数，若失败返回 */
	if (args_analyse_process(argc, argv, args, 0) < 0)
		return -1;
	/* 帮助选项：打印信息并退出（保留原有块） */
	if (args[0].existed()) {
		cout << endl << "Version : V2025.12.23" << endl << endl;
		cout << "args_list:" << endl;
		args_analyse_print(args);
		usage(argv[0]);
		return 0;
	}
	/* --checkname 必须提供 */
	if (args[2].existed() == 0 || argc == 1) {
		cout << endl << "Version : V2025.12.23" << endl << endl;
		cout << "args_list:" << endl;
		args_analyse_print(args);
		usage(argv[0]);
		cout << "==============" << endl;
		cout << "错误提示信息：" << endl;
		cout << "==============" << endl;
		cout << "参数[--checkname]必须选择" << endl << endl << endl;
		return -1;
	}
	const string cfgfile = args[4].get_string();
	const string checkname_raw = args[2].get_string();

	config_file_tools sCfg(cfgfile.c_str());
	if (sCfg.is_read_succeeded() == false) {
		cout << "配置文件[" << cfgfile << "]读取不成功" << endl;
		return -1;
	}
	string group_name = checkname_raw;
	normalize_group_name(group_name);
	vector<string> errors;
	vector<string> chain;
	collect_include_chain_recursive(sCfg, group_name, chain); /* 递归实现（无 visited 检查） */

	if (args[3].existed()) {//checkcfg_only
		for (size_t i = 0; i < chain.size(); ++i) {
			print_section_header(chain[i]);
			checkcfg_only_print_one_group(sCfg, chain[i], errors);
		}
		print_section_header(group_name);
		checkcfg_only_print_one_group(sCfg, group_name, errors);
	}
	else { //非 checkcfg_only 模式，分下面几个阶段完成
		/* 1)读取能变化的三要素 */
		string exe_style, name_list, cmd_style;
		get_item_string_from_cfg(sCfg, group_name, "exe_style", exe_style);
		get_item_string_from_cfg(sCfg, group_name, "name_list", name_list);
		get_item_string_from_cfg(sCfg, group_name, "cmd_style", cmd_style);
		trim(exe_style); trim(name_list); trim(cmd_style);
		/* 2)items /timeout /max_output / tc_* */
		int items_num = 0;
		if (!get_item_int_from_cfg(sCfg, group_name, "items_num", items_num) || items_num < 1 || items_num > 1024) {
			errors.push_back("items_num 不存在或不合法");
			items_num = 0;
		}
		int items_begin = 1;
		int items_end = items_num;
		int tmp = 0;
		if (get_item_int_from_cfg(sCfg, group_name, "items_begin", tmp) && tmp >= 1 && tmp <= 1024)
			items_begin = tmp;
		if (get_item_int_from_cfg(sCfg, group_name, "items_end", tmp) && tmp >= 1 && tmp <= 1024)
			items_end = tmp;
		if (items_num > 0) {
			if (items_begin < 1)
				items_begin = 1;
			if (items_begin > items_num) 
				items_begin = items_num;
			if (items_end < items_begin) 
				items_end = items_begin;
			if (items_end > items_num) 
				items_end = items_num;
		}
		int timeout_sec = 3;
		int max_output_len = 256;
		vector<string> tmp_errors;
		cft_get_int_or_default_with_error(sCfg, group_name, "timeout", 1, 3600, 3, 1, timeout_sec, tmp_errors);
		cft_get_int_or_default_with_error(sCfg, group_name, "max_output_len", 1, 65536, 256, 1, max_output_len, tmp_errors);
		for (size_t i = 0; i < tmp_errors.size(); ++i)
			errors.push_back(tmp_errors[i]);
		string tc_trim, tc_display;
		get_item_string_from_cfg(sCfg, group_name, "tc_trim", tc_trim);
		get_item_string_from_cfg(sCfg, group_name, "tc_display", tc_display);
		trim(tc_trim); 
		trim(tc_display);
		if (tc_trim.empty())
			tc_trim = "right";
		if (tc_display.empty()) 
			tc_display = "none";

		int tc_lineskip = 0, tc_lineoffset = 0, tc_ignoreblank = 0, tc_not_ignore_linefeed = 0, tc_maxdiff = 0, tc_maxline = 0;
		{
			vector<string> tmp_errors;
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_lineskip", 0, 100, 0, 0, tc_lineskip, tmp_errors);
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_lineoffset", -100, 100, 0, -100, tc_lineoffset, tmp_errors);
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_ignoreblank", 0, 1, 0, 0, tc_ignoreblank, tmp_errors);
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_not_ignore_linefeed", 0, 1, 0, 0, tc_not_ignore_linefeed, tmp_errors);
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_maxdiff", 0, 100, 0, 0, tc_maxdiff, tmp_errors);
			cft_get_int_or_default_with_error(sCfg, group_name, "tc_maxline", 0, 10000, 0, 0, tc_maxline, tmp_errors);
		    for (size_t i = 0; i < tmp_errors.size(); ++i)
			    errors.push_back(tmp_errors[i]);
		}
		/* 3) 路径项 */
		string single_exe_dirname, multi_exe_main_dirname, multi_exe_sub_dirname, redirection_data_dirname;
		string stu_exe_name, demo_exe_name, pipe_get_input_data_exe_name, pipe_data_file;
		get_item_string_from_cfg(sCfg, group_name, "stu_exe_name", stu_exe_name);
		get_item_string_from_cfg(sCfg, group_name, "demo_exe_name", demo_exe_name);
		get_item_string_from_cfg(sCfg, group_name, "pipe_get_input_data_exe_name", pipe_get_input_data_exe_name);
		get_item_string_from_cfg(sCfg, group_name, "pipe_data_file", pipe_data_file);
		get_item_string_from_cfg(sCfg, group_name, "single_exe_dirname", single_exe_dirname);
		get_item_string_from_cfg(sCfg, group_name, "multi_exe_main_dirname", multi_exe_main_dirname);
		get_item_string_from_cfg(sCfg, group_name, "multi_exe_sub_dirname", multi_exe_sub_dirname);
		get_item_string_from_cfg(sCfg, group_name, "redirection_data_dirname", redirection_data_dirname);
		trim(stu_exe_name);
		trim(demo_exe_name);
		trim(pipe_get_input_data_exe_name); 
		trim(pipe_data_file);
		trim(single_exe_dirname);
		trim(multi_exe_main_dirname); 
		trim(multi_exe_sub_dirname); 
		trim(redirection_data_dirname);
		ensure_dir_endslash(single_exe_dirname);
		ensure_dir_endslash(multi_exe_main_dirname);
		ensure_dir_endslash(multi_exe_sub_dirname);
		ensure_dir_endslash(redirection_data_dirname);
		/* 必要文件存在性检查（只做关键项） */
		if (demo_exe_name.empty() || !file_exists_by_stat(demo_exe_name))
			errors.push_back("demo_exe_name 不存在或文件不可访问");
		if (cmd_style == "pipe") {
			if (pipe_get_input_data_exe_name.empty() || !file_exists_by_stat(pipe_get_input_data_exe_name))
				errors.push_back("pipe_get_input_data_exe_name 不存在或不可访问");
			if (pipe_data_file.empty() || !file_exists_by_stat(pipe_data_file))
				errors.push_back("pipe_data_file 不存在或不可访问");
		}
		if (cmd_style == "redirection") {
			if (redirection_data_dirname.empty() || !dir_exists_by_stat(redirection_data_dirname))
				errors.push_back("redirection_data_dirname 不存在或不可访问");
		}

		/* 4)学生列表 */
		vector<stu_info> stus;
		if (exe_style != "none") {		
			if (name_list == "database") {
				db_connect_and_query_students(sCfg, stus, errors);
			}
			else {
				load_student_list_from_txt(name_list, stus, errors);
			}
			/* 在生成demo之前，先按学号从小到大排序，并输出重复提示 */
			if (errors.empty()) {
				sort_students_by_stuno(stus);
				print_duplicate_students_if_any(stus);
			}
		}
		if (!errors.empty()) {
			;//存在错误不继续往下走
		}
		else {
			/* 5) 读取测试项列表 */
			vector<string> items;
			collect_items_values(sCfg, group_name, cmd_style, items_num, items, errors);
			if (errors.empty() && (int)items.size() != items_num) {
				ostringstream oss;
				oss << "实际读取的测试项数量(" << items.size()
					<< ")与配置的 items_num(" << items_num << ")不一致";
				errors.push_back(oss.str());
			}

			/* 校验 items_begin/items_end 范围 */
			if (errors.empty()) {
				if (items_begin < 1 || items_begin >(int)items.size()) {
					ostringstream oss;
					oss << "items_begin[" << items_begin << "] 超出 items 实际范围[1.." << items.size() << "]";
					errors.push_back(oss.str());
				}
				if (items_end < 1 || items_end >(int)items.size()) {
					ostringstream oss;
					oss << "items_end[" << items_end << "] 超出 items 实际范围[1.." << items.size() << "]";
					errors.push_back(oss.str());
				}
				if (items_begin > items_end) {
					ostringstream oss;
					oss << "items_begin[" << items_begin << "] > items_end[" << items_end << "]";
					errors.push_back(oss.str());
				}
			}

			/* 6) 先运行 demo（参考答案）每个测试项一次，缓存输出（避免每个学生重复跑） */
			string demo_begin_time = now_timestr_for_screen();
			vector<string> demo_outputs;
			demo_outputs.resize((size_t)items_num);
			int demo_cnt_ok = 0;
			int demo_cnt_create_timer_failed = 0, demo_cnt_popen_failed = 0, demo_cnt_start_timer_failed = 0;
			int demo_cnt_timeout = 0, demo_cnt_max_output = 0, demo_cnt_killed = 0;
			for (int i = items_begin; i <= items_end; ++i) {
				const string& itemv = items[(size_t)(i - 1)];
				string cmd_demo = build_demo_exec_cmd(cmd_style, demo_exe_name, pipe_get_input_data_exe_name, pipe_data_file, redirection_data_dirname, itemv);
				class_exe_runner r(cmd_demo, get_basename_from_path(demo_exe_name), max_output_len, timeout_sec);
				int rc = r.run();
				CheckExec_Errno eno = r.get_errno();
				if (rc == 0 && eno == CheckExec_Errno::ok) {
					demo_cnt_ok++;
					demo_outputs[(size_t)(i - 1)] = r.get_output();
				}
				else {
					if (eno == CheckExec_Errno::create_timer_id_failed) 
						demo_cnt_create_timer_failed++;
					else if (eno == CheckExec_Errno::popen_faliled)
						demo_cnt_popen_failed++;
					else if (eno == CheckExec_Errno::start_timer_failed)
						demo_cnt_start_timer_failed++;
					else if (eno == CheckExec_Errno::timeout)
						demo_cnt_timeout++;
					else if (eno == CheckExec_Errno::max_output)
						demo_cnt_max_output++;
					else if (eno == CheckExec_Errno::killed_by_callback) 
						demo_cnt_killed++;
					ostringstream oss;
					oss << "参考程序运行失败: item=" << i;
					errors.push_back(oss.str());
					break;
				}
			}
			string demo_end_time = now_timestr_for_screen();
			cout << demo_end_time << " 参考答案生成情况" << endl;
			cout << string(70, '=') << endl;
			cout << "参考exe文件：" << demo_exe_name << endl;
			if (demo_cnt_ok > 0) {
				cout << "测试运行情况：正确运行=" << demo_cnt_ok << endl;
			}
			else if (demo_cnt_timeout > 0) {
				cout << "测试运行情况：超时=" << demo_cnt_timeout << endl;
			}
			else if (demo_cnt_max_output > 0) {
				cout << "测试运行情况：超过输出上限=" << demo_cnt_max_output << endl;
			}
			else if (demo_cnt_killed > 0) {
				cout << "测试运行情况：死循环=" << demo_cnt_killed << endl;
			}
			else if (demo_cnt_popen_failed > 0) {
				cout << "测试运行情况：管道方式打开失败=" << demo_cnt_popen_failed << endl;
			}
			else if (demo_cnt_start_timer_failed > 0) {
				cout << "测试运行情况：启动定时器失败=" << demo_cnt_start_timer_failed << endl;
			}
			else if (demo_cnt_create_timer_failed > 0) {
				cout << "测试运行情况：定时器创建失败=" << demo_cnt_create_timer_failed << endl;
			}
			else {
				cout << "测试运行情况：正确运行=0" << endl;
			}
			cout << "时间：" << demo_begin_time << " - " << demo_end_time << endl;
			cout << string(70, '=') << endl << endl;
			if (exe_style == "none")
				return 0;//exe_style none 不继续跑学生直接返回即可

			/* 7) 生成输出文件名 */
			string namelist_tag = (name_list == "database") ? "database" : "txt";
			string fname = "check-result-2452487-" + now_timestr_for_filename()
				+ "-" + exe_style + "-" + cmd_style + "-" + namelist_tag + "-" + get_basename_from_path(stu_exe_name) + ".xls";
			ofstream out(fname.c_str(), ios::out | ios::binary);
			if (!out.is_open()) {
				errors.push_back("无法创建输出文件: " + fname);
			}
			else {
				/* 8) 输出头部 */
				out << "exe_style\t" << exe_style << "\n";
				out << "cmd_style\t" << cmd_style << "\n";
				out << "name_list\t" << name_list << "\n";
				out << "stu_exe_name\t" << stu_exe_name << "\n";
				out << "statrt_time\t" << now_timestr_for_filename() << "\n\n";
				/* 9) 表头 */
				out << "序号\t课号\t学号\t姓名\t正确运行\t定时器创建失败\t管道方式打开失败\t启动定时器失败\t超时\t超过输出上限\t死循环\tTC通过总数";
				for (int i = items_begin; i <= items_end; ++i) {
					if (cmd_style == "pipe") {
						string g = items[(size_t)(i - 1)];
						trim(g);
						if (!g.empty() && !(g.front() == '[' && g.back() == ']'))
							g = "[" + g + "]";
						out << "\t" << g;
					}
					else if (cmd_style == "redirection") {
						out << "\t" << items[(size_t)(i - 1)];
					}
					else if (cmd_style == "main_with_arguments") {
						out << "\t" << items[(size_t)(i - 1)];
					}
					else {
						out << "\t" << "第" << i << "次";
					}
				}
				out << "\n";
				/*10)：对每个学生跑item*/
				for (size_t si = 0; si < stus.size(); ++si) {
					const stu_info& stu = stus[si];
					string stu_begin_time = now_timestr_for_screen();
					int cnt_ok = 0, cnt_create_timer_failed = 0, cnt_popen_failed = 0, cnt_start_timer_failed = 0;
					int cnt_timeout = 0, cnt_max_output = 0, cnt_killed = 0;
					int tc_pass_total = 0;
					/* 每项结果：1/0 或 / */
					vector<string> item_marks;
					item_marks.resize((size_t)(items_end - items_begin + 1), "0");
					/* multi模式 exe 路径容错，避免因为目录结构差异导致一直判无exe从而全0 */
					vector<string> tried_paths;
					string stu_exe_fullpath = resolve_student_exe_fullpath(exe_style, single_exe_dirname,
						multi_exe_main_dirname, multi_exe_sub_dirname, stu.cno, stu.stuno, stu_exe_name, &tried_paths);

					bool has_exe = (!stu_exe_fullpath.empty());

					/* 仅当没找到exe时打印尝试过的路径，便于定位配置/目录结构问题 */
					if (!has_exe && args[1].existed()) {
						cout << "提示：未找到学生exe，已尝试路径如下：" << endl;
						for (size_t ti = 0; ti < tried_paths.size(); ++ti) {
							cout << "  " << tried_paths[ti] << endl;
						}
					}
					for (int i = items_begin; i <= items_end; ++i) {
						size_t idx = (size_t)(i - items_begin);
						if (!has_exe || exe_style == "none") {
							item_marks[idx] = "0";
							continue;
						}
						const string& itemv = items[(size_t)(i - 1)];

						string cmd_stu = build_student_exec_cmd(cmd_style, stu_exe_fullpath,
							pipe_get_input_data_exe_name, pipe_data_file, redirection_data_dirname, itemv);
						class_exe_runner rr(cmd_stu, get_basename_from_path(stu_exe_fullpath), max_output_len, timeout_sec);
						int rrc = rr.run();
						CheckExec_Errno eno = rr.get_errno();
						if (rrc == 0 && eno == CheckExec_Errno::ok) {
							cnt_ok++;

							const string& demo_out = demo_outputs[(size_t)(i - 1)];
							const string stu_out = rr.get_output();
							if (args[1].existed()) {//--debug调试							
								cout << "demo_out" << demo_out;
								cout << "stu_out" << stu_out;
							}			
							istringstream iss_demo(demo_out);
							istringstream iss_stu(stu_out);
							txt_compare tc(iss_demo, iss_stu,tc_trim,tc_display,tc_lineskip, tc_lineoffset, tc_maxdiff, tc_maxline,
								(tc_ignoreblank != 0), (tc_not_ignore_linefeed != 0), false);
							int diff_count = tc.compare(false); // 不输出，仅通过返回值判断
							if (diff_count == 0) {
								tc_pass_total++;
								item_marks[idx] = "1";//正确，对应置1
							}
							else {
								item_marks[idx] = "0";
							}
						}
						else {/* 错误分类计数 */							
							if (eno == CheckExec_Errno::create_timer_id_failed) 
								cnt_create_timer_failed++;
							else if (eno == CheckExec_Errno::popen_faliled) 
								cnt_popen_failed++;
							else if (eno == CheckExec_Errno::start_timer_failed) 
								cnt_start_timer_failed++;
							else if (eno == CheckExec_Errno::timeout) 
								cnt_timeout++;
							else if (eno == CheckExec_Errno::max_output) 
								cnt_max_output++;
							else if (eno == CheckExec_Errno::killed_by_callback)
								cnt_killed++;
							item_marks[idx] = "0";
						}
					}
					string stu_end_time = now_timestr_for_screen();
					string marks_line;
					for (size_t k = 0; k < item_marks.size(); ++k) {
						if (!item_marks[k].empty())
							marks_line.push_back(item_marks[k][0]);
						else
							marks_line.push_back('0');
					}
					cout << stu_end_time << " 学生答案生成情况" << endl;
					cout << string(70, '=') << endl;
					cout << "序号：" << (si + 1) << " 学号：" << stu.stuno << " / 姓名：" << stu.name << " / 课号： " << stu.cno << endl;
					int total_items = items_end - items_begin + 1;
					if (!has_exe || exe_style == "none") {
						cout << "测试运行情况：无exe=/" << endl;
					}
					else if (cnt_ok == total_items) {
						cout << "测试运行情况：正确运行=" << cnt_ok << endl;
					}
					else if (cnt_max_output == total_items) {
						cout << "测试运行情况：超过输出上限=" << cnt_max_output << endl;
					}
					else if (cnt_timeout == total_items) {
						cout << "测试运行情况：超时=" << cnt_timeout << endl;
					}
					else if (cnt_killed == total_items) {
						cout << "测试运行情况：死循环=" << cnt_killed << endl;
					}
					else if (cnt_popen_failed == total_items) {
						cout << "测试运行情况：管道方式打开失败=" << cnt_popen_failed << endl;
					}
					else if (cnt_start_timer_failed == total_items) {
						cout << "测试运行情况：启动定时器失败=" << cnt_start_timer_failed << endl;
					}
					else if (cnt_create_timer_failed == total_items) {
						cout << "测试运行情况：定时器创建失败=" << cnt_create_timer_failed << endl;
					}
					else {
						cout << "测试运行情况：正确运行=" << cnt_ok << endl;
					}
					cout << "逐项正确性：" << marks_line << endl;
					cout << "tc正确项数：" << tc_pass_total << endl;
					cout << "时间：" << stu_begin_time << " - " << stu_end_time << endl;
					cout << string(70, '=') << endl << endl;
					/* 写 xls */
					out << (si + 1) << "\t"<< excel_escape_stuno(stu.cno) << "\t"
						<< excel_escape_stuno(stu.stuno) << "\t"<< stu.name << "\t";
					if (!has_exe || exe_style == "none") {
						out << "/\t/\t/\t/\t/\t/\t/\t0";
					}
                    else {
						out << cnt_ok << "\t"
							<< cnt_create_timer_failed << "\t"
							<< cnt_popen_failed << "\t"
							<< cnt_start_timer_failed << "\t"
							<< cnt_timeout << "\t"
							<< cnt_max_output << "\t"
							<< cnt_killed << "\t"
							<< tc_pass_total;
					}

					for (size_t k = 0; k < item_marks.size(); ++k)
						out << "\t" << item_marks[k];
					out << "\n";
				}
				out << "\n";
				cout << now_timestr_for_screen() << " 检查结果文件[" << fname << "]已生成." << endl;
			}
		}
	}//not checkcfg_only

	/* 最后统一输出错误块，无论有没有checkcfg_only */
	if (!errors.empty()) {
		cerr << "[--严重错误--] 配置文件存在下列的错误：" << endl;//先打印这行，maxline比对到这里
		for (size_t i = 0; i < errors.size(); ++i) {
			cerr << errors[i] << endl;
		}
		return -1;
	}

	return 0;
}

