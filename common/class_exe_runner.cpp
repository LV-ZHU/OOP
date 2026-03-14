/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/class_exe_runner.h"
#include <cstdlib>
#include <cstdio>
#include <Windows.h>
using namespace std;

/* 定义使用新版定时器（基于CreateTimerQueueTimer）还是旧版定时器（基于timeSetEvent） */
#define TIMER_NEW_VERSION 1

#if TIMER_NEW_VERSION
static VOID CALLBACK timer_queue_callback(PVOID ExtParameter, BOOLEAN TimerOrWaitFired);
#else
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")
static void CALLBACK timer_queue_callback(UINT uTimerID, UINT uMsg, DWORD ExtParameter, DWORD dw1, DWORD dw2);
#endif

/***************************************************************************
  函数名称：is_valid_image_name_for_taskkill
  功    能：检查进程镜像名是否合法（用于 taskkill 命令）
  输入参数：const string& image_name：进程镜像名（如 demo.exe）
  返 回 值：bool - true 表示合法，false 表示非法
  说    明：不能为空，不能包含路径分隔符（\ 或 /），仅允许纯文件名，用于防止命令注入攻击
***************************************************************************/
static bool is_valid_image_name_for_taskkill(const string& image_name)
{
	if (image_name.empty())
		return false;
	if (image_name.find('\\') != string::npos || image_name.find('/') != string::npos)
		return false;
	return true;
}

/***************************************************************************
  函数名称：taskkill_image
  功    能：强制终止指定进程镜像名的所有进程
  输入参数：const string& image_name：进程镜像名（如 demo.exe）
  返 回 值：无
  说    明：
***************************************************************************/
static void taskkill_image(const string& image_name)
{
	if (!is_valid_image_name_for_taskkill(image_name))
		return;

	char cmd_taskkill[1024];
	sprintf(cmd_taskkill, "taskkill /f /t /im \"%s\" 1>nul 2>&1", image_name.c_str());
	system(cmd_taskkill);
}

/***************************************************************************
  函数名称：构造函数
  功    能：初始化 exe 运行器对象
  输入参数：
    const string& full_exec_cmd：完整执行命令（如 "demo.exe < input.txt"）
    const string& exec_name：可执行文件名（如 "demo.exe"，用于 taskkill）
    int max_output_len：最大输出长度限制（字节数）
    int timeout_second：超时时间（秒）
  返 回 值：无
  说    明：
***************************************************************************/
class_exe_runner::class_exe_runner(const string& full_exec_cmd, const string& exec_name, int max_output_len, int timeout_second)
	: full_exec_cmd(full_exec_cmd),
	exec_name(exec_name),
	cfg_timeout(timeout_second),
	max_output_len(max_output_len),
	fp_exe(NULL),
	time_count(0),
	timeout_flag(0),
	timer_id(NULL),
	time_tick(0),
	begin_time(0),
	end_time(0),
	eno(CheckExec_Errno::ok),
	output()
{
}

/***************************************************************************
  函数名称：析构函数
  功    能：清理资源，确保进程被终止、定时器被停止、管道被关闭
  输入参数：无
  返 回 值：无
  说    明：
***************************************************************************/
class_exe_runner::~class_exe_runner()
{
	this->stop_timer();

	if (this->eno != CheckExec_Errno::ok) {
		taskkill_image(this->exec_name);
	}

	if (this->fp_exe) {
		_pclose(this->fp_exe);
		this->fp_exe = NULL;
	}
}

/***************************************************************************
  函数名称：reset
  功    能：重置对象状态，以便重新运行
  输入参数：无
  返 回 值：无
  说    明：
***************************************************************************/
void class_exe_runner::reset()
{
	this->time_count = 0;
	this->timeout_flag = 0;
	this->fp_exe = NULL;
	this->timer_id = NULL;
	this->time_tick = 0;
	this->begin_time = 0;
	this->end_time = 0;
	this->eno = CheckExec_Errno::ok;
	this->output.clear();
}

/***************************************************************************
  函数名称：get_full_cmd_exec
  功    能：获取完整执行命令
  输入参数：无
  返 回 值：string - 完整执行命令字符串
  说    明：返回构造时传入的 full_exec_cmd
***************************************************************************/
string class_exe_runner::get_full_cmd_exec() const
{
	return this->full_exec_cmd;
}

/***************************************************************************
  函数名称：get_errno
  功    能：获取错误码
  输入参数：无
  返 回 值：CheckExec_Errno - 枚举类型错误码
  说    明：
    ok：正常运行结束
    timeout：超时
    max_output：输出超长
    killed_by_callback：被定时器回调强制终止
    create_timer_id_failed：创建定时器失败
    start_timer_failed：启动定时器失败
    popen_faliled：管道打开失败
***************************************************************************/
CheckExec_Errno class_exe_runner::get_errno() const
{
	return this->eno;
}

/***************************************************************************
  函数名称：get_output
  功    能：获取程序输出内容
  输入参数：无
  返 回 值：string - 程序的标准输出内容（可能被截断）
  说    明：
***************************************************************************/
string class_exe_runner::get_output() const
{
	return this->output;
}

/***************************************************************************
  函数名称：get_running_time
  功    能：获取程序实际运行时间（秒）
  输入参数：无
  返 回 值：double - 运行时间（秒），精度取决于系统性能计数器
  说    明：
***************************************************************************/
double class_exe_runner::get_running_time() const
{
	if (this->time_tick == 0)
		return 0.0;
	return double(this->end_time - this->begin_time) / double(this->time_tick);
}

/***************************************************************************
  函数名称：timeout_process
  功    能：定时器回调处理函数的核心逻辑（每秒调用一次）
  输入参数：void* ExtParameter：指向 class_exe_runner 对象的指针
  返 回 值：无
  说    明：
***************************************************************************/
void timeout_process(void* ExtParameter)
{
	class_exe_runner* my_exe = (class_exe_runner*)ExtParameter;
	if (my_exe == NULL)
		return;

	if (++my_exe->time_count >= my_exe->cfg_timeout) {
		my_exe->eno = CheckExec_Errno::timeout;
		my_exe->timeout_flag = 1;

		int delta = my_exe->cfg_timeout < 5 ? my_exe->cfg_timeout * 2 : 5;
		if (my_exe->time_count >= my_exe->cfg_timeout + delta) {
			my_exe->stop(CheckExec_Errno::killed_by_callback);
		}
	}
}

/***************************************************************************
  函数名称：timer_queue_callback（新版定时器）
  功    能：定时器队列回调函数（CreateTimerQueueTimer）
  输入参数：
  返 回 值：无
  说    明：
***************************************************************************/
#if TIMER_NEW_VERSION
static VOID CALLBACK timer_queue_callback(PVOID ExtParameter, BOOLEAN TimerOrWaitFired)
{
	(void)TimerOrWaitFired;
	timeout_process(ExtParameter);
}

/***************************************************************************
  函数名称：timer_queue_callback（旧版定时器）
  功    能：回调函数（timeSetEvent）
  输入参数：
  返 回 值：无
  说    明：
***************************************************************************/
#else
static void CALLBACK timer_queue_callback(UINT uTimerID, UINT uMsg, DWORD ExtParameter, DWORD dw1, DWORD dw2)
{
	(void)uTimerID; (void)uMsg; (void)dw1; (void)dw2;
	timeout_process((void*)ExtParameter);
}
#endif

/***************************************************************************
  函数名称：start_timer
  功    能：启动定时器（每秒触发一次回调）
  输入参数：无
  返 回 值：0：成功
  说    明：
***************************************************************************/
int class_exe_runner::start_timer()
{
#if TIMER_NEW_VERSION
	HANDLE hTimer = NULL;
	if (CreateTimerQueueTimer(&hTimer, NULL, timer_queue_callback, this, 1000, 1000, NULL) == 0) {
		this->eno = CheckExec_Errno::create_timer_id_failed;
		return -1;
	}
	this->timer_id = (void*)hTimer;
#else
	MMRESULT tid = timeSetEvent(1000, 100, timer_queue_callback, (DWORD_PTR)this, TIME_PERIODIC);
	if (tid == NULL) {
		this->eno = CheckExec_Errno::create_timer_id_failed;
		return -1;
	}
	this->timer_id = (void*)tid;
#endif

	this->time_count = 0;
	this->timeout_flag = 0;

	LARGE_INTEGER freq;
	LARGE_INTEGER begin;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&begin);

	this->time_tick = freq.QuadPart;
	this->begin_time = begin.QuadPart;
	this->end_time = begin.QuadPart;

	return 0;
}

/***************************************************************************
  函数名称：stop_timer
  功    能：停止并销毁定时器
  输入参数：无
  返 回 值：无
  说    明：设置 timer_id = NULL，防止重复释放
***************************************************************************/
void class_exe_runner::stop_timer()
{
#if TIMER_NEW_VERSION
	if (this->timer_id != NULL) {
		DeleteTimerQueueTimer(NULL, (HANDLE)this->timer_id, NULL);
		this->timer_id = NULL;
	}
#else
	if (this->timer_id != NULL) {
		timeKillEvent((MMRESULT)this->timer_id);
		this->timer_id = NULL;
	}
#endif
}

/***************************************************************************
  函数名称：stop
  功    能：停止程序运行，清理资源
  输入参数：CheckExec_Errno no：停止原因（错误码）
  返 回 值：0（总是成功）
  说    明：
***************************************************************************/
int class_exe_runner::stop(CheckExec_Errno no)
{
	this->stop_timer();

	LARGE_INTEGER endt;
	QueryPerformanceCounter(&endt);
	this->end_time = endt.QuadPart;

	this->eno = no;

	if (this->eno != CheckExec_Errno::ok) {
		taskkill_image(this->exec_name);
	}

	if (this->fp_exe) {
		_pclose(this->fp_exe);
		this->fp_exe = NULL;
	}

	return 0;
}

/***************************************************************************
  函数名称：run
  功    能：运行程序并捕获输出（阻塞式）
  输入参数：无
  返 回 值：0：正常结束
  说    明：
***************************************************************************/
int class_exe_runner::run()
{
	if ((this->fp_exe = _popen(this->full_exec_cmd.c_str(), "rb")) == NULL) {
		this->eno = CheckExec_Errno::popen_faliled;
		return -1;
	}

	if (this->start_timer() < 0) {
		this->eno = CheckExec_Errno::start_timer_failed;
		return -1;
	}

	this->output.clear();
	this->output.reserve(this->max_output_len > 0 ? (size_t)this->max_output_len : 0);

	signed char ch;
	int ch_num = 0;

	while ((ch = fgetc(this->fp_exe)) != EOF) {
		this->output.push_back((char)ch);
		++ch_num;

		if (this->timeout_flag != 0 || this->eno == CheckExec_Errno::timeout) {
			this->stop(CheckExec_Errno::timeout);
			return -1;
		}

		if (ch_num >= this->max_output_len) {
			this->stop(CheckExec_Errno::max_output);
			return -1;
		}
	}

	if (this->eno == CheckExec_Errno::killed_by_callback)
		return -1;

	this->stop(CheckExec_Errno::ok);
	return 0;
}