/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include "../include/cmd_hdc_tools.h"
#include "../include/cmd_console_tools.h"
using namespace std;

#if !HDC_SERIES_BY_TEACHER	//未定义的情况下才打开条件编译

/* 允许添加需要的头文件 */

extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow(); //VS中此处会有波浪线，不影响正常使用

/* 已定义的静态全局变量 */
static HWND hWnd = GetConsoleWindow();
static HDC hdc = NULL;
static const double PI = 3.14159;
static int _BgColor_, _FgColor_, _Width_, _High;
static int _Delay_ms;

/* 允许适度使用静态全局变量，但仅限static，即外部不可见 */

/* 此处允许添加自定义函数，但仅限static，即外部不可见 */
/***************************************************************************
  函数名称：
  功    能：统一的角度转换函数，转换为系统角度
  输入参数：const int angle：输入角度
  返 回 值：
  说    明：把参数角度转为系统角度（系统0°在最下方，顺时针递增），并加上差值
***************************************************************************/
static inline int hdc_angle_change_to_system(const int angle, int difference, bool isClockWise)
{
	int a = angle + difference; // 先加上差值
	a = (a % 360 + 360) % 360; // 保证在0~359之间
	// 顺时针直接用，逆时针用360减去
	if (isClockWise)
		return a;
	else
		return (360 - a) % 360;
}

/***************************************************************************
  函数名称：swap_two
  功    能：指针法交换两个数
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
static inline void swap_two(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
/***************************************************************************
  函数名称：from_smally_to_bigy
  功    能：根据y坐标，将三个点(vx1,vy1), (vx2,vy2), (vx3,vy3)从小到大排序，引用法
  输入参数：三个点的x和y坐标的引用
  返 回 值：无
  说    明：确保当y交换时，其对应的x也一起交换
***************************************************************************/
static inline void from_smally_to_bigy(int& x1, int& y1, int& x2, int& y2, int& x3, int& y3)
{
	// 比较点1和点2
	if (y1 > y2) {
		swap_two(&x1, &x2); // 交换x
		swap_two(&y1, &y2); // 交换y
	}
	// 比较点1和点3
	if (y1 > y3) {
		swap_two(&x1, &x3); // 交换x
		swap_two(&y1, &y3); // 交换y
	}
	// 经过前两步，点1已经是y坐标最小的点。现在只需比较点2和点3
	if (y2 > y3) {
		swap_two(&x2, &x3); // 交换x
		swap_two(&y2, &y3); // 交换y
	}
}

/* 下面给出了几个基本函数的完整实现，不要改动 */
/***************************************************************************
  函数名称：
  功    能：初始化
  输入参数：const int bgcolor：背景色
			const int fgcolor：前景色
			const int width  ：屏幕宽度（点阵）
			const int high   ：屏幕高度（点阵）
  返 回 值：
  说    明：
***************************************************************************/
void hdc_init(const int bgcolor, const int fgcolor, const int width, const int high)
{
	/* 先释放，防止不release而再次init（hdc_release可重入） */
	hdc_release();

	/* 窗口init后，用一个静态全局量记录，后续hdc_cls()会用到 */
	_BgColor_ = bgcolor;
	_FgColor_ = fgcolor;
	_Width_ = width;
	_High = high;

	hdc = GetDC(hWnd);

	cct_setcursor(CCT_CURSOR_INVISIBLE);
	cct_setcolor(bgcolor, fgcolor);
	cct_setfontsize("点阵字体", 16);
	cct_setconsoleborder(width / 8 + !!(width % 8), high / 16 + !!(high % 16)); //将点阵的宽度及高度转换为特定字体的行列数，!!的含义：如果不是8/16的倍数，行列多+1
	cct_cls();
}

/***************************************************************************
  函数名称：
  功    能：释放画图资源
  输入参数：
  返 回 值：
  说    明：可重入
***************************************************************************/
void hdc_release()
{
	if (hdc) {
		ReleaseDC(hWnd, hdc);
		hdc = NULL;
		cct_setcursor(CCT_CURSOR_VISIBLE_NORMAL);
	}
}

/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
void hdc_set_delay(int ms)
{
	if (ms > 0)
		_Delay_ms = ms;
	else
		_Delay_ms = 0;
}

/***************************************************************************
  函数名称：
  功    能：设置画笔颜色，传入RGB值
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
void hdc_set_pencolor(const int RGB_value)
{
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, RGB_value);
}

/***************************************************************************
  函数名称：
  功    能：设置画笔颜色，传入RGB三色，值0-255
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
void hdc_set_pencolor(const unsigned char red, const unsigned char green, const unsigned char blue)
{
	hdc_set_pencolor(RGB(red, green, blue));
}

/***************************************************************************
  函数名称：
  功    能：清除屏幕上现有的图形
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
void hdc_cls()
{
	/* 发现一定要换一种颜色初始化才能清除像素点，找到更简便方法的同学可以通知我 */
	hdc_init(_BgColor_, (_FgColor_ + 1) % 16, _Width_, _High);

	hdc_init(_BgColor_, _FgColor_, _Width_, _High);

	/* 部分机器上运行demo时，会出现hdc_cls()后第一根线有缺线的情况，加延时即可
	   如果部分机器运行还有问题，调高此延时值 */
	Sleep(30);
}

/***************************************************************************
  函数名称：
  功    能：在(x,y)位置处用指定颜色画出一个像素点
  输入参数：const int x：x坐标，左上角为(0,0)
			const int y：y坐标，左上角为(0,0)
  返 回 值：
  说    明：颜色直接用当前设定，可以在调用处指定
***************************************************************************/
static inline void hdc_base_point(const int x, const int y)
{
	MoveToEx(hdc, x - 1, y - 1, NULL);
	LineTo(hdc, x, y);
	if (_Delay_ms > 0)
		Sleep(_Delay_ms);
}

/***************************************************************************
  函数名称：
  功    能：在(x1,y1)-(x2,y2)之间画出一个像素点的连线
  输入参数：const int x1：起点x坐标，左上角为(0,0)
			const int y1：起点y坐标，左上角为(0,0)
			const int x2：终点y坐标，左上角为(0,0)
			const int y2：终点y坐标，左上角为(0,0)
  返 回 值：
  说    明：颜色直接用当前设定
***************************************************************************/
static inline void hdc_base_line(const int x1, const int y1, const int x2, const int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	if (_Delay_ms > 0)
		Sleep(_Delay_ms);
}

/***************************************************************************
  函数名称：
  功    能：在(x,y)位置处用指定颜色画一个指定粗细的点(用画实心圆来模拟)
  输入参数：const int x			：x坐标，左上角为(0,0)
			const int y			：y坐标，左上角为(0,0)
			const int thickness	：点的粗细，下限位1，上限不限(如过大会导致不完全填充)（有缺省值）
			const int RGB_value	：点的颜色（有缺省值）
  返 回 值：
  说    明：改进运行效率
***************************************************************************/
void hdc_point(const int x, const int y, const int thickness, const int RGB_value)
{
	int tn = thickness;
	if (tn < 1)
		tn = 1;		//处理下限
	/* 不卡点直径的上限，但是单上限超过一定大小是，画出的圆部分位置未实心 */

	const int tn_end = (tn <= 1) ? 1 : tn / 2;

	int angle, level;
	int old_x1 = INT_MAX, old_y1 = INT_MAX, x1, y1;
	int count = 0;

	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);

	/* 用画圆的方式模拟粗点 */
	for (level = 1; level <= tn_end; level++) {
		for (angle = 0; angle <= 360; angle++) {
			/* 注意，系统的坐标轴，0°在圆的最下方 */
			x1 = x + (int)(level * sin(angle * PI / 180));
			y1 = y - (int)(level * cos(angle * PI / 180));

			/* 当半径很小时，角度变化不会引起int型的x1/y1变化，因此加判断语句，避免重复画同一像素点 */
			if (x1 != old_x1 || y1 != old_y1) {
				old_x1 = x1;
				old_y1 = y1;
				hdc_base_point(x1, y1);
				++count;
			}
		}
	}
#if 0
	/* 放开此注释，可以看到一个粗点用了几个像素点组成 */
	cct_gotoxy(0, 41);
	printf("count=%d           ", count);
	getchar();
#endif
}

/* -------------------------------------------------------------------------
	给出下列函数的实现，函数名及参数表不准改动
	1、不需要调用系统的其他有关图形操作的函数
	2、下列函数的实现都基于hdc_base_point/hdc_base_line函数的组合
	3、想一想，是不是所有函数都需要给出独立的实现过程？应该先实现哪些函数？
	4、填充是实现中最复杂的部分
	5、系统的角度与函数参数中的角度含义相差180°
   ------------------------------------------------------------------------- */

   /***************************************************************************
	 函数名称：
	 功    能：画线段
	 输入参数：const int x1		：起点的x
			   const int y1		：起点的y
			   const int x2		：终点的x
			   const int y2		：终点的y
			   const int thickness	：线段的粗细（有缺省值）
			   const int RGB_value	：线段的颜色（有缺省值）
	 返 回 值：
	 说    明：本函数可以用基于hdc_point的组合（速度慢）
					 也可以用hdc_base_point/hdc_base_line的组合（速度快）
   ***************************************************************************/
void hdc_line(const int x1, const int y1, const int x2, const int y2, const int thickness, const int RGB_value)
{	
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value); // 如果指定了颜色值，则设置当前画笔颜色

	int tn = thickness;
	if (tn < 1)
		tn = 1;		//处理下限
	if (x1 == x2 && y1 == y2) { // 如果起点和终点相同，则直接画一个粗点
		hdc_point(x1, y1, thickness); 
		return;
	}
	for (int offset_width = -tn / 2; offset_width <= tn / 2; offset_width++)
		hdc_base_line(x1 + offset_width, y1, x2 + offset_width, y2);// 水平偏移
	for (int offset_height = -tn / 2; offset_height <= tn / 2; offset_height++)
		hdc_base_line(x1, y1 + offset_height, x2, y2 + offset_height);// 竖直偏移，光保留水平偏移有些线条显示会不连续，故加入竖直偏移
}

/***************************************************************************
  函数名称：
  功    能：给出三点的坐标，画一个三角形
  输入参数：const int x1		：第1个点的x
			const int y1		：第1个点的y
			const int x2		：第2个点的x
			const int y2		：第2个点的y
			const int x3		：第3个点的x
			const int y3		：第3个点的y
			bool filled			：是否需要填充（有缺省值）
			const int thickness	：边的粗细（有缺省值）
			const int RGB_value	：颜色（有缺省值）
  返 回 值：
  说    明：不判断三点是否共线，如果共线，划出一条直线即可
***************************************************************************/
void hdc_triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, bool filled, const int thickness, const int RGB_value)
{
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);
	if (thickness > 0) {//和demo一样先画边框
		hdc_line(x1, y1, x2, y2, thickness, RGB_value);
		hdc_line(x2, y2, x3, y3, thickness, RGB_value);
		hdc_line(x3, y3, x1, y1, thickness, RGB_value);
	}
	if (filled) {
		int x[3] = { x1, x2, x3 };//原来的x1-x3为const，不可变，故存入数组
		int y[3] = { y1, y2, y3 };
		from_smally_to_bigy(x[0], y[0], x[1], y[1], x[2], y[2]); //将y坐标从小到大排序
		// 为了确保顶点是尖的，先单独画出三个顶点
		hdc_base_point(x[0], y[0]);
		hdc_base_point(x[1], y[1]);
		hdc_base_point(x[2], y[2]);
		// 绘制上半部分 (从 y[0] 到 y[1])
		if (y[1] > y[0]) {
			double m_long = (double)(x[2] - x[0]) / (y[2] - y[0]);  // 长边(P0->P2)的斜率倒数
			double m_upper = (double)(x[1] - x[0]) / (y[1] - y[0]); // 上短边(P0->P1)的斜率倒数

			for (int t = y[0]; t <= y[1]; t++) { 
				int x_start = (int)(x[0] + m_long * (t - y[0]) + 0.5);// 加0.5进行四舍五入，和人民币转大写一样
				int x_end = (int)(x[0] + m_upper * (t - y[0]) + 0.5);
				hdc_base_line(x_start, t, x_end, t);
			}
		}
		// 绘制下半部分 (从 y[1] 到 y[2]，逻辑和上面一样)
		if (y[2] > y[1]) {
			double m_long = (double)(x[2] - x[0]) / (y[2] - y[0]);  // 长边(P0->P2)的斜率倒数
			double m_lower = (double)(x[2] - x[1]) / (y[2] - y[1]); // 下短边(P1->P2)的斜率倒数

			for (int t = y[1]; t <= y[2]; t++) { 
				int x_start = (int)(x[0] + m_long * (t - y[0]) + 0.5);
				int x_end = (int)(x[1] + m_lower * (t - y[1]) + 0.5);
				hdc_base_line(x_start, t, x_end, t);
			}
		}	
	}
}

/***************************************************************************
  函数名称：
  功    能：给出左上角坐标及宽度、高度，画出一个长方形
  输入参数：const int left_up_x			：左上角x
			const int left_up_y			：左上角y
			const int width				：宽度
			const int high				：高度
			const int rotation_angles	：以左上角为支点，与x轴的旋转倾角（向下转为正）
			bool filled					：是否需要填充（有缺省值）
			const int thickness			：边的粗细（有缺省值）
			const int RGB_value			：颜色（有缺省值）
  返 回 值：
  说    明：
***************************************************************************/
void hdc_rectangle(const int left_up_x, const int left_up_y, const int width, const int high, const int rotation_angles, const bool filled, const int thickness, const int RGB_value)
{
	const int w = (width < 1 ? 1 : width);
	const int h = (high < 1 ? 1 : high);
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);

	int px[4], py[4];
	const double angle_rad = rotation_angles * PI / 180.0;
	const double cos_a = cos(angle_rad);
	const double sin_a = sin(angle_rad);
	// 定义矩形四个顶点相对于旋转中心(左上角)的局部坐标，放入数组
	const int corners_x[4] = { 0, w - 1, w - 1, 0 };
	const int corners_y[4] = { 0, 0, h - 1, h - 1 };

	for (int i = 0; i < 4; i++) {
		px[i] = left_up_x + (int)(corners_x[i] * cos_a - corners_y[i] * sin_a + 0.5);//旋转公式加旋转坐标平移
		py[i] = left_up_y + (int)(corners_x[i] * sin_a + corners_y[i] * cos_a + 0.5);
	}
	if (filled) {//填充模式：用两个三角形拼成矩形
		hdc_triangle(px[0], py[0], px[1], py[1], px[3], py[3], true, 1, INVALID_RGB);
		hdc_triangle(px[1], py[1], px[2], py[2], px[3], py[3], true, 1, INVALID_RGB);
	}
	else {// 不填充模式：直接画四条边
		hdc_line(px[0], py[0], px[1], py[1], thickness, INVALID_RGB);
		hdc_line(px[1], py[1], px[2], py[2], thickness, INVALID_RGB);
		hdc_line(px[2], py[2], px[3], py[3], thickness, INVALID_RGB);
		hdc_line(px[3], py[3], px[0], py[0], thickness, INVALID_RGB);
	}
}

/***************************************************************************
  函数名称：
  功    能：给出左上角坐标及边长，画出一个长方形
  输入参数：const int left_up_x			：左上角x
			const int left_up_y			：左上角y
			const int length			：边长
			const int rotation_angles	：以左上角为支点，与x轴的旋转倾角（向下转为正）（有缺省值）
			const bool filled			：是否需要填充（有缺省值）
			const int thickness			：边的粗细（有缺省值）
			const int RGB_value			：颜色（有缺省值）
  输入参数：
  返 回 值：
  说    明：正方形是特殊的长方形
***************************************************************************/
void hdc_square(const int left_up_x, const int left_up_y, const int length, const int rotation_angles, const bool filled, const int thickness, const int RGB_value)
{
	hdc_rectangle(left_up_x, left_up_y, length, length, rotation_angles, filled, thickness, RGB_value);
}

/***************************************************************************
  函数名称：
  功    能：按照给出的参数画一段圆弧
  输入参数：const int point_x		：圆心的x
			const int point_y		：圆心的y
			const int radius		：半径
			const int angle_begin	：起始角度（以圆的最上方为0°，顺时针为正，注意：与系统坐标差了180°!!!）（有缺省值）
			const int angle_end		：结束角度（同上）（有缺省值）
			const int thickness		：粗细（有缺省值）
			const int RGB_value		：颜色（有缺省值）
  返 回 值：
  说    明：
***************************************************************************/
void hdc_arc(const int point_x, const int point_y, const int radius, const int angle_begin, const int angle_end, const int thickness, const int RGB_value)
{
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);

	int a_b = hdc_angle_change_to_system(angle_begin, 180, true);
	int a_e = hdc_angle_change_to_system(angle_end, 180, true);	
	while (a_e <= a_b)
		a_e += 360;//保证终止角度大于起始角度
	int old_x = -1, old_y = -1; // 用-1表示还没开始
	for (int angle = a_b; angle <= a_e; angle++) {
		int x = point_x - (int)(radius * sin(angle * PI / 180.0) + 0.5);
		int y = point_y + (int)(radius * cos(angle * PI / 180.0) + 0.5);
		if (old_x != -1 && old_y != -1) 
			hdc_line(old_x, old_y, x, y, thickness, INVALID_RGB); // 用线连接，点函数画出来太离散
		old_x = x;
		old_y = y;
	}
}

/***************************************************************************
  函数名称：
  功    能：按照给出的参数画一个扇形
  输入参数：const int point_x		：圆心的x
			const int point_y		：圆心的y
			const int radius		：半径
			const int angle_begin	：起始角度（以圆的最上方为0°，顺时针为正，注意：与系统坐标差了180°!!!）（有缺省值）
			const int angle_end		：结束角度（同上）（有缺省值）
			const bool filled		：是否需要填充（有缺省值）
			const int thickness		：粗细（有缺省值）
			const int RGB_value		：颜色（有缺省值）
  输入参数：
  返 回 值：
  说    明：当起始/结束角度差值为360的倍数时，不画两条边
***************************************************************************/
void hdc_sector(const int point_x, const int point_y, const int radius, const int angle_begin, const int angle_end, const bool filled, const int thickness, const int RGB_value)
{
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);

	hdc_arc(point_x, point_y, radius, angle_begin, angle_end, thickness, INVALID_RGB);//先画轮廓

	bool is_full_circle = false;
	if ((angle_end - angle_begin) % 360 == 0) 
		is_full_circle = true;//当起始/结束角度差值为360的倍数时，不画两条边

	if (!is_full_circle) {
		int sys_b = hdc_angle_change_to_system(angle_begin, 180, true);
		int sys_e = hdc_angle_change_to_system(angle_end, 180, true);
		int x_b = point_x - (int)(radius * sin(sys_b * PI / 180.0) );
		int y_b = point_y + (int)(radius * cos(sys_b * PI / 180.0) );
		int x_e = point_x - (int)(radius * sin(sys_e * PI / 180.0) );
		int y_e = point_y + (int)(radius * cos(sys_e * PI / 180.0) );

		hdc_line(point_x, point_y, x_b, y_b, thickness, INVALID_RGB); // 画起始半径
		hdc_line(point_x, point_y, x_e, y_e, thickness, INVALID_RGB); // 画终止半径
	}
	// 填充扇形
	if (filled) {
		int a1 = (angle_begin % 360 + 360) % 360;
		int a2 = (angle_end % 360 + 360) % 360;
		if (is_full_circle || a1 == a2)
			a2 = a1 + 360;
		else if (a1 > a2)
			a2 += 360;
		double rad_prev =(hdc_angle_change_to_system(a1, 180, true)) * PI / 180.0;
		int x_prev = point_x - (int)(radius * sin(rad_prev) );//previous的x坐标
		int y_prev = point_y + (int)(radius * cos(rad_prev) );//previous的y坐标
		for (int a = a1 + 1; a <= a2; a++) {
			double rad_now = (hdc_angle_change_to_system(a, 180, true))*PI / 180.0;
			int x_now = point_x - (int)(radius * sin(rad_now) );
			int y_now = point_y + (int)(radius * cos(rad_now) );
			if (x_now != x_prev || y_now != y_prev) {
				hdc_triangle(point_x, point_y, x_prev, y_prev, x_now, y_now, true, 0, INVALID_RGB); // 三角扇填充
				hdc_line(point_x, point_y, x_now, y_now, 1, INVALID_RGB);//否则会留缝隙
				x_prev = x_now;
				y_prev = y_now;
			}
		}
	}
}

/***************************************************************************
  函数名称：
  功    能：按照给出的参数画一个圆
  输入参数：const int point_x		：圆心的x
			const int point_y		：圆心的y
			const int radius		：半径
			const bool filled		：是否需要填充（有缺省值）
			const int thickness		：粗细（有缺省值）
			const int RGB_value		：颜色（有缺省值）
  返 回 值：
  说    明：
***************************************************************************/
void hdc_circle(const int point_x, const int point_y, const int radius, const bool filled, const int thickness, const int RGB_value)
{
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);
	hdc_arc(point_x, point_y, radius, 0, 360, thickness, INVALID_RGB);//除去填充情况外的操作已完成

	if (filled){
		for (int angle = 0; angle <= 180; angle ++) {//由于point_x * 2 - x画180度即可填满整个圆
			int x = (int)(point_x - (double)(radius * sin(angle * PI / 180)) + 0.5);
			int y = (int)(point_y + (double)(radius * cos(angle * PI / 180)) + 0.5);
			hdc_line(point_x * 2 - x, y, x, y, thickness + 1, RGB_value);	//用圆的弦填充
		}
	}
}

/***************************************************************************
  函数名称：
  功    能：按照给出的参数画一个椭圆
  输入参数：const int point_x			：圆心的x
			const int point_y			：圆心的y
			const int radius_a			：平行于X轴的半径
			const int radius_b			：平行于Y轴的半径
			const int rotation_angles	：以圆心为支点，与x轴的旋转倾角（向下转为正）（有缺省值）
			const bool filled			：是否需要填充（有缺省值）
			const int thickness			：粗细（有缺省值）
			const int RGB_value			：颜色（有缺省值）
  返 回 值：
  说    明：
***************************************************************************/
void hdc_ellipse(const int point_x, const int point_y, const int radius_a, const int radius_b, const int rotation_angles, const bool filled, const int thickness, const int RGB_value)
{
	if (RGB_value != INVALID_RGB)
		hdc_set_pencolor(RGB_value);

	const double cos1 = cos(rotation_angles * PI / 180.0);
	const double sin1 = sin(rotation_angles * PI / 180.0);

	// 画椭圆轮廓（用线段连接，类似hdc_arc）
	int old_x = -1, old_y = -1;
	for (int t = 0; t <= 360; t++) {
		const double x1 = radius_a * cos(t * PI / 180.0);
		const double y1 = radius_b * sin(t * PI / 180.0);
		const int x_final = point_x + (int)(x1 * cos1 - y1 * sin1 + 0.5);
		const int y_final = point_y + (int)(x1 * sin1 + y1 * cos1 + 0.5);
		if (old_x != -1 && old_y != -1) {
			hdc_line(old_x, old_y, x_final, y_final, thickness, INVALID_RGB);
		}
		old_x = x_final;
		old_y = y_final;
	}

	// 填充椭圆（类似hdc_sector）
	if (filled) {
		int x_prev = point_x + (int)(radius_a * cos1 + 0.5); // t=0时的点
		int y_prev = point_y + (int)(radius_a * sin1 + 0.5);
		for (int t = 1; t <= 360; t++) {
			double angle = t * PI / 180.0;
			double x0 = radius_a * cos(angle);
			double y0 = radius_b * sin(angle);
			int x_now = point_x + (int)(x0 * cos1 - y0 * sin1 + 0.5);
			int y_now = point_y + (int)(x0 * sin1 + y0 * cos1 + 0.5);

			if (x_now != x_prev || y_now != y_prev) {
				hdc_triangle(point_x, point_y, x_prev, y_prev, x_now, y_now, true, 0, INVALID_RGB); // 三角扇填充
				hdc_line(point_x, point_y, x_now, y_now, 1, INVALID_RGB);//否则会留缝隙
				x_prev = x_now; 
				y_prev = y_now; 
			}
		}
	}
}

#endif !HDC_SERIES_BY_TEACHER
