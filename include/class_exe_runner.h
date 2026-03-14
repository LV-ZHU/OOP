/* 2452487 信安 胡中芃 */
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
using namespace std;

enum class CheckExec_Errno {
	ok = 0,
	create_timer_id_failed,
	popen_faliled,
	start_timer_failed,
	timeout,
	max_output,
	killed_by_callback,
	max
};

void timeout_process(void* ExtParameter);//必须声明在类前面，否则编译报错

class class_exe_runner {
private:
	string full_exec_cmd;
	string exec_name;
	int cfg_timeout;
	int max_output_len;
	FILE* fp_exe;
	int time_count;
	int timeout_flag;
	void* timer_id;
	long long time_tick;
	long long begin_time;
	long long end_time;
	CheckExec_Errno eno;
	string output;
	int start_timer();
	void stop_timer();
	int stop(CheckExec_Errno eno);
	friend void timeout_process(void* ExtParameter);
public:
	class_exe_runner(const string& full_exec_cmd, const string& exec_name, int max_output_len, int timeout_second);
	~class_exe_runner();
	class_exe_runner(const class_exe_runner&) = delete;
	class_exe_runner& operator=(const class_exe_runner&) = delete;
	int run();
	void reset();
	CheckExec_Errno get_errno() const;
	double get_running_time() const; 
	string get_full_cmd_exec() const;
	string get_output() const;
};