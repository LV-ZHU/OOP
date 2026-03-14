#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <string>
#include <conio.h>
#include <Windows.h>
using namespace std;

#define TIMER_NEW_VERSION	1

#if TIMER_NEW_VERSION

//新版本定时器（CreateTimerQueueTimer函数及相关配套函数，适用于多线程）
//仅<Windows.h>即可，不需要其它头文件

#else

//旧版本定时器（timeSetEvent函数及相关配套函数，不适用多线程）
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")

#endif

/* 定义各种返回类型 */
enum class CheckExec_Errno {
	ok = 0,
	create_timer_id_failed,		//创建定时器ID失败
	popen_faliled,		//管道方式打开可执行文件失败
	start_timer_failed,	//启动定时器失败
	timeout,				//超时
	max_output,			//达到设定的输出上限（死循环输出，反正肯定不对了）
	killed_by_callback, //死循环（超时且fgetc阻塞）
	max
};

/***************************************************************************
  函数名称：
  功    能：CheckExec_Errno的输出
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
ostream& operator<<(ostream& out, const CheckExec_Errno& eno)
{
	switch (eno) {
		case CheckExec_Errno::ok:
			out << "正确运行";
			break;
		case CheckExec_Errno::create_timer_id_failed:
			out << "定时器创建失败";
			break;
		case CheckExec_Errno::popen_faliled:
			out << "管道方式打开失败";
			break;
		case CheckExec_Errno::start_timer_failed:
			out << "启动定时器失败";
			break;
		case CheckExec_Errno::timeout:
			out << "超时";
			break;
		case CheckExec_Errno::max_output:
			out << "超过输出上限";
			break;
		case CheckExec_Errno::killed_by_callback:
			out << "死循环";
			break;
		default:
			out << "未知的错误";
			break;
	}//end of switch
	return out;
}

class st_CheckExec {
protected:
	/* 初始化的4个参数 */
	string full_exec_cmd;	//完整的执行命令（用于_popen）
	string exec_name;		//exe文件名（用于taskkill）
	int    cfg_timeout;		//设置的超时（秒）
	int    max_output_len;	//读取输出的最大长度

	FILE* fp_exe;
	int    time_count;

#if TIMER_NEW_VERSION
	HANDLE		timer_id; //定时器ID
#else
	MMRESULT     timer_id; //定时器ID
#endif
	LARGE_INTEGER time_tick;
	LARGE_INTEGER begin_time;
	LARGE_INTEGER end_time;

	CheckExec_Errno eno;  //错误号（不能叫errno）

	int  start_timer();
	void stop_timer();
	int  stop(CheckExec_Errno eno);
public:
	ostringstream msg; //存放输出
	int              running();
	double           get_running_time() const;
	string          get_full_cmd_exec() const;
	CheckExec_Errno get_errno() const;
	int              reset(); //重置，进行下次运行
	st_CheckExec(const string& full_exec_cmd, const string& exec_name, int max_output_len, int timeout_second);
	~st_CheckExec();

	/* 回调函数声明为友元 */
#if TIMER_NEW_VERSION
	friend void CALLBACK timeout_process(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
#else
	friend void CALLBACK timeout_process(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif
};

#if TIMER_NEW_VERSION
static void CALLBACK timeout_process(PVOID ExtParameter, BOOLEAN TimerOrWaitFired);
#else
static void CALLBACK timeout_process(UINT uTimerID, UINT uMsg, DWORD ExtParameter, DWORD dw1, DWORD dw2);
#endif

/***************************************************************************
  函数名称：
  功    能：回调函数因为已经限定了类型，不能是成员函数
  输入参数：dwUser：启动定时器时传入了this指针
  返 回 值：
  说    明：设置了1秒定时器，因此每秒会触发一次，
***************************************************************************/
#if TIMER_NEW_VERSION
static void CALLBACK timeout_process(PVOID ExtParameter, BOOLEAN TimerOrWaitFired)
#else
static void CALLBACK timeout_process(UINT uTimerID, UINT uMsg, DWORD ExtParameter, DWORD dw1, DWORD dw2)
#endif
{
	st_CheckExec* my_exe = (st_CheckExec*)ExtParameter;

	/* 判断是否超时时间到（置标记，具体杀任务等操作到回调函数外面处理） */
	if (++my_exe->time_count >= my_exe->cfg_timeout) {
		my_exe->eno = CheckExec_Errno::timeout;

		/* 死机防止：
			前提：如果程序运行后没有任何输出，会导致running函数中while循环的fgetc函数阻塞（不返回），此时即使置了timeout，也无法进入running函数的while循环中，导致无法结束
					例：一个需要输入的程序，用 cmd_style = normal 方式运行，导致没有任何输入，会阻塞fgetc
			处理：在本回调函数中调用taskkill
			注意：在回调函数尽量不执行长时间操作，否则可能会带来阻塞或延迟风险，因此做一个预防性处理，只有超时后running不处理，才在本函数中taskkill
		*/
		int delta = my_exe->cfg_timeout < 5 ? my_exe->cfg_timeout * 2 : 5; //预防性处理：超时<5秒，两倍时间，>=5，加5秒
		if (my_exe->time_count >= my_exe->cfg_timeout + delta) {
			//cout << "timeout = " << my_exe->time_count << " " << my_exe->msg.str().length() << " " << my_exe->eno << endl;
			my_exe->stop(CheckExec_Errno::killed_by_callback);
		}
	}
}

/***************************************************************************
  函数名称：
  功    能：启动定时器
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int st_CheckExec::start_timer()
{
#if TIMER_NEW_VERSION
	//	this->timer_queue = CreateTimerQueue();
	//	if (this->timer_queue == NULL) {
	//		this->eno = CheckExec_Errno::create_timer_queue_failed;
	//		return -1;
	//	}
		/* 参数解释：
		   this->timer_id  ：定时器句柄
		   NULL            ：定时器队列句柄（NULL表示和默认队列相关，this->timer_queue则表示新建队列）
		   timeout_process ：回调函数
		   this            ：回调函数的附加参数（一个任意类型的指针）
		   0               ：定时器延迟启动时间（ms）
		   1000            ：定时器间隔（ms）
		   NULL            ：标记位

		   ChangeTimerQueueTimer：改变定时器设置
		   DeleteTimerQueueTimer：删除定时器
		*/
	if (CreateTimerQueueTimer(&this->timer_id, NULL, timeout_process, this, 1000, 1000, NULL) == 0) {
		this->eno = CheckExec_Errno::create_timer_id_failed;
		return -1;
	}
#else
	/* 启动定时器（间隔1000ms，精度100ms） */
	this->timer_id = timeSetEvent(1000, 100, timeout_process, (DWORD_PTR)this, TIME_PERIODIC);
	if (timer_id == NULL) {
		this->eno = CheckExec_Errno::create_timer_id_failed;
		return -1;
	}
#endif

	/* 计时器清0 */
	this->time_count = 0;

	/* 开始时间计数 */
	QueryPerformanceFrequency(&this->time_tick);	//获得硬件支持的高精度计数器的频率
	QueryPerformanceCounter(&this->begin_time);	//获得初始硬件定时器计数

	return 0;
}

/***************************************************************************
  函数名称：
  功    能：删除定时器
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
void st_CheckExec::stop_timer()
{
#if TIMER_NEW_VERSION
	if (this->timer_id != NULL) {
		DeleteTimerQueueTimer(NULL, this->timer_id, NULL);
		this->timer_id = NULL;
	}
#else
	if (this->timer_id != NULL) {
		timeKillEvent(this->timer_id);
		this->timer_id = NULL;
	}
#endif
	return;
}

/***************************************************************************
  函数名称：
  功    能：删除定时器，杀掉正在运行的任务
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int st_CheckExec::stop(CheckExec_Errno no)
{
	this->stop_timer();
	QueryPerformanceCounter(&this->end_time);//获得终止硬件定时器计数

	this->eno = no;

	/* 如果非正常结束，用 killtask 命令杀掉超时进程
		1、exe文件名为不带路径的裸文件名
		2、如果之后的 _pclose() 不返回（程序死掉），说明 taskkill 可能未成功 */
	if (this->eno != CheckExec_Errno::ok) {
		char cmd_taskkill[512];
		sprintf(cmd_taskkill, "taskkill /f /t /im %s 1>nul 2>&1", this->exec_name.c_str());
		system(cmd_taskkill);
	}

	/* 关闭文件（必须在taskkill之后，如果_plose不返回，说明task_kill可能未执行成功） */
	if (this->fp_exe) {
		_pclose(this->fp_exe);
		this->fp_exe = NULL;
	}
	return 0;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
string st_CheckExec::get_full_cmd_exec() const
{
	return this->full_exec_cmd;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
CheckExec_Errno st_CheckExec::get_errno() const
{
	return this->eno;
}

/***************************************************************************
  函数名称：
  功    能：获取定时器的运行时间(ms)
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
double st_CheckExec::get_running_time() const
{
	return double(this->end_time.QuadPart - this->begin_time.QuadPart) / double(time_tick.QuadPart);
}

/***************************************************************************
  函数名称：
  功    能：如果有必要，可以reset后重复运行
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
int st_CheckExec::reset()
{
	this->time_count = 0;
	this->fp_exe = NULL;
	this->timer_id = NULL;
	this->eno = CheckExec_Errno::ok;
	this->msg.str("");

	return 0;
}

/***************************************************************************
  函数名称：
  功    能：构造函数
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
st_CheckExec::st_CheckExec(const string& full_exec_cmd, const string& exec_name, int max_output_len, int timeout_second)
{
	this->full_exec_cmd = full_exec_cmd;
	this->exec_name = exec_name;
	this->max_output_len = max_output_len;
	this->cfg_timeout = timeout_second;

	this->time_count = 0;
	this->fp_exe = NULL;
	this->timer_id = NULL;
	this->eno = CheckExec_Errno::ok;
	this->msg.str("");
}

/***************************************************************************
  函数名称：
  功    能：析构函数
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
st_CheckExec::~st_CheckExec()
{
	if (this->timer_id != NULL)
		this->stop_timer();

	/* 如果非正常结束，生成 killtask 命令，杀掉超时进程（exe文件名一定要正确） */
	if (this->eno != CheckExec_Errno::ok) {
		char cmd_taskkill[512];
		sprintf(cmd_taskkill, "taskkill /f /t /im %s 1>nul 2>&1", this->exec_name.c_str());
		system(cmd_taskkill);
	}

	/* 关闭文件（必须在taskkill之后） */
	if (this->fp_exe) {
		_pclose(this->fp_exe);
		this->fp_exe = NULL;
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
int st_CheckExec::running()
{
	/* 第1步：以 r 方式打开要运行的exe文件
		r ：行结束符是\n
		rb：行结束符是\r\n */
	if ((this->fp_exe = _popen(this->full_exec_cmd.c_str(), "rb")) == NULL) {
		this->eno = CheckExec_Errno::popen_faliled;
		return -1;
	}

	/* 启动定时器 */
	if (this->start_timer() < 0) { //启动定时器(1秒)
		this->msg << "启动定时器错误" << endl;
		this->eno = CheckExec_Errno::start_timer_failed;
		return -1;
	}

	/* 第2步：从fp_exe中读数据（模拟exe的输出），放入 */
	signed char ch;
	int  ch_num = 0;

	/* 循环未结束表示程序运行未结束，因此回调函数会在循环中被触发，回调函数执行完成后，仍然停留在循环中 */
	while ((ch = fgetc(this->fp_exe)) != EOF) {
		this->msg << ch; //循环输出到msg中
		++ch_num;
#if 0 //如果需要，打开后看程序输出信息
		//		cout << "num=" << ch_num << " ch=" << int(ch) << endl;
		if (ch_num % 256 == 0) { //没256个输出打印一次
			cout << ch_num << ' ';
			cout << msg.str() << endl;
		}
#endif
		/* 超时：emo已在回调函数中设置，回调函数执行完成后会执行到此处 */
		if (this->eno == CheckExec_Errno::timeout) {
			this->stop(CheckExec_Errno::timeout);
			return -1;
		}

		/* 超长 */
		if (ch_num >= this->max_output_len) {
			this->stop(CheckExec_Errno::max_output);
			return -1;
		}
	} //end of while

	/* 执行到此，有两种情况，分开处理
		1、程序已执行完成（读到了EOF），正常结束，停止定时器即可
		2、因为死循环/fgetc阻塞（没有任何输出导致fgetc不返回），这种情况能执行到此处，说明是回调函数杀的进程 */
	if (this->eno == CheckExec_Errno::killed_by_callback)
		return -1;

	/* 正常结束 */
	this->stop(CheckExec_Errno::ok);
	return 0;
	return 0;
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
 ***************************************************************************/
int main()
{
	st_CheckExec example_set[] = {
		/* 直接运行，正确 */
		{"D:\\25261-term\\参考exe\\第03模块-循环-九九乘法表.exe", "第03模块-循环-九九乘法表.exe", 1024, 1},
		/* 直接运行，超长错误 */
		{"D:\\25261-term\\参考exe\\第03模块-循环-九九乘法表.exe", "第03模块-循环-九九乘法表.exe", 128, 1},
		/* 管道方式运行，正确 */
		{"D:\\25261-term\\get_input_data.exe D:\\25261-term\\测试数据\\3-b3-pipe-data.txt [3-b3-22] | D:\\25261-term\\参考exe\\3-b3-demo-浮点数分解.exe","3-b3-demo-浮点数分解.exe", 256, 3},
		/* 重定向方式运行，正确 */
		{"D:\\25261-term\\参考exe\\3-b3-demo-浮点数分解.exe < D:\\25261-term\\测试数据\\3-b3-09.txt","3-b3-demo-浮点数分解.exe", 256, 3},
		/* 命令行参数方式运行（一重循环改进版，10秒以内） */
		{"D:\\25261-term\\参考exe\\1953-多种解.exe --3", "1953-多种解.exe", 30720, 20},
		/* 命令行参数方式运行（一重循环基本型，20秒以上），超长错误 */
		{"D:\\25261-term\\参考exe\\1953-多种解.exe --1", "1953-多种解.exe", 1024, 10},
		/* 命令行参数方式运行（一重循环基本型，20秒以上），超时错误 */
		{"D:\\25261-term\\参考exe\\1953-多种解.exe --1", "1953-多种解.exe", 30720, 3},
		/* 直接运行，显示菜单后等待输入，用正常方式执行会出现死循环错误（15秒后callback中taskkill） */
		{"D:\\25261-term\\参考exe\\1953-多种解.exe", "1953-多种解.exe", 30720, 10},
	};

	for (int i = 0; i < sizeof(example_set) / sizeof(st_CheckExec); i++) {
		/* 执行一个命令 */
		cout << endl << "--------------------------------------------" << endl;
		cout << "运行命令：" << example_set[i].get_full_cmd_exec() << endl;
		example_set[i].running();

		cout << "程序状态：" << example_set[i].get_errno() << endl;
		cout << "运行时间：" << example_set[i].get_running_time() << endl;

		/* 正确结束则打印输出内容 */
		if (example_set[i].get_errno() == CheckExec_Errno::ok) {
			cout << "程序输出：" << endl << example_set[i].msg.str() << endl << endl;
		}
		else {
			cout << "程序已输出长度：" << example_set[i].msg.str().length() << endl << endl;
		}
		cout << "--------------------------------------------" << endl;
	}

	return 0;
}
