/* 2452487 信安 胡中芃 */
#include <iostream>
#include <iomanip>
#include <conio.h>
#include <Windows.h>
#include "../include/cmd_hdc_tools.h"
#include "../include/cmd_console_tools.h"
#include "90-01-b1-hanoi.h"
#include "90-01-b1-hanoi_const_value.h"
using namespace std;

/* ----------------------------------------------------------------------------------

     本文件功能：
    1、存放被 hanoi_main.cpp 中根据菜单返回值调用的各菜单项对应的执行函数

     本文件要求：
    1、不允许定义外部全局变量（const及#define不在限制范围内）
    2、允许定义静态全局变量（具体需要的数量不要超过文档显示，全局变量的使用准则是：少用、慎用、能不用尽量不用）
    3、静态局部变量的数量不限制，但使用准则也是：少用、慎用、能不用尽量不用
    4、按需加入系统头文件、自定义头文件、命名空间等

   ----------------------------------------------------------------------------------- */

int buzhoushu;            // 总移动步数，步骤计数器
int towers[3][MAX_LAYER]; // A,B,C三柱现有圆盘的编号
// towers[0][]表示A柱，towers[1][]表示B柱，towers[2][]表示C柱
int top[3];   // A,B,C三柱现有圆盘的数量，栈顶指针，指向即将出栈位置
// top[0]表示A柱，top[1]表示B柱，top[2]表示C柱
int delay = 0;            // 延时

/***************************************************************************
   函数名称：时间延迟
   功    能：延时/按回车单步演示
   输入参数：delay:延时
   返 回 值：空
   说    明：delay == 0为按回车单步演示，否则Sleep相应时间
***************************************************************************/
void delay_step(int delay)
{
    if (delay == 0)
        while (1) {
            if (_getch() == 13)
                break;
        }
    else
        Sleep(delay);
}

/***************************************************************************
   函数名称：输入提示
   功    能：显示输入提示信息并完成对应初始化操作
   输入参数：n:层数 src、tmp、dst:起始、中间、目标柱 sel:菜单项
   返 回 值：空
   说    明：其他功能：数组初始化；步骤数初始化；菜单7的时候提前交换中间柱和目标柱
 ***************************************************************************/
void prompt(int* n, char* src, char* dst, char* tmp, int sel)
{
    while (1) {
        cout << "请输入汉诺塔的层数(1-10)" << endl;
        cin >> *n;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(65536, '\n');
            continue;
        }
        cin.ignore(65536, '\n');
        if (*n >= 1 && *n <= MAX_LAYER)
            break;
    }//end of while
    while (1) {
        cout << "请输入起始柱(A-C)" << endl;
        cin >> *src;
        cin.ignore(65536, '\n');
        if (*src >= 'a' && *src <= 'c')//转大写
            *src -= 32;
        if (*src >= 'A' && *src <= 'C')
            break;
    }//end of while
    while (1) {
        cout << "请输入目标柱(A-C)" << endl;
        cin >> *dst;
        cin.ignore(65536, '\n');
        if (*dst >= 'a' && *dst <= 'c')//转大写
            *dst -= 32;
        if (*dst == *src) {//处理起始和目标柱相同的情况
            cout << "目标柱(" << *dst << ")不能与起始柱(" << *dst << ")相同" << endl;
            continue;
        }
        if (*dst >= 'A' && *dst <= 'C')
            break;
    }//end of while
    int Delay_max = 0; // 定义延时上限，初始化为0
    if (sel == '7' || sel == '8' || sel == '9')
        Delay_max = 20; // 菜单7-9延时上限为20ms
    if (sel == '4')
        Delay_max = 200;// 菜单4延时上限为200ms
    if (sel == '4' || sel == '7' || sel == '8' || sel == '9')
    {
        while (1)
        {
            cout << "请输入移动速度(0-" << Delay_max << ": 0-按回车单步演示 1-"
                << Delay_max << ":延时1-" << Delay_max << "ms) ";
            cin >> delay;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(65536, '\n');
                continue;
            }
            if (delay > Delay_max || delay < 0)
                continue;
            break;
        }
    }

    buzhoushu = 0;//总移动步数初始化（否则按多次会累加，导致错误）
    // 数组初始化  
    if (sel == '3' || sel == '4' || sel == '7' || sel == '8' || sel == '9') {
        for (int i = 0; i < 3; i++) {// 清空所有柱子数据（否则按多次会累加，导致错误）
            top[i] = 0;
            for (int j = 0; j < MAX_LAYER; j++)
                towers[i][j] = 0;
        }
        for (int i = *n; i >= 1; i--)
            towers[*src - 'A'][top[*src - 'A']++] = i;//把起始柱对应圆盘编号完成初始化 
    }

    *tmp = 'A' + 'B' + 'C' - *src - *dst;// 计算中间柱
    if (sel == '4' || sel == '6' || sel == '7' || sel == '8' || sel == '9') {//清屏后输出提示 
        cct_cls();
        cct_gotoxy(Status_Line_X, Status_Line_Y);
        cout << "从 " << *src << " 移动到 " << *dst << "，共 " << *n << " 层 ";
        if (sel == '4' || sel == '8')
            cout << "，延时设置为 " << delay << "ms";
        if (sel == '8')
            cout << "（前7步，后面自动变为0ms）";
        if (sel == '7' && *n % 2 == 0) {// 菜单7且为偶数层时交换tmp和dst的角色（偶数层首次是源柱到中间柱）
            *tmp = *dst;
            *dst = 'A' + 'B' + 'C' - *src - *dst;
        }
    }
}

/***************************************************************************
   函数名称：唯一一个、核心递归函数（形参数量可以不断添加）
   功    能：经典汉诺塔结构，核心思路为通过调用move函数来实现不同菜单项的功能
   输入参数：n:层数 src、tmp、dst:起始、中间、目标柱 sel:菜单项
   返 回 值：空
   说    明：可以改为n == 0的时候return，但需要多执行n == 0的情况，效率低
***************************************************************************/
void hanoi(int n, char src, char tmp, char dst, char sel)
{
    if (n == 1)
        move(1, src, dst, sel);
    else {
        hanoi(n - 1, src, dst, tmp, sel);
        move(n, src, dst, sel);
        hanoi(n - 1, tmp, src, dst, sel);
    }
}

/***************************************************************************
   函数名称：移动
   功    能：执行不同菜单功能的总函数
   输入参数：n:层数 src、dst:起始、目标柱 sel:菜单项
   返 回 值：空
   说    明：（1）菜单1/2直接按格式cout即可；菜单3/4/7/8/9均需要改变内部数组元素；
                  菜单7只执行第一次；菜单8/9调用图形移动；菜单3/4/8/9需要横向移动；菜单4/8/9需要纵向移动
             （2）内部数组元素改变时不需要考虑原来的柱子上的数值，同5-b7
***************************************************************************/
void move(int n, char src, char dst, char sel)
{
    if (sel == '1')
        cout << n << "# " << src << "---->" << dst << endl;
    if (sel == '2') {
        buzhoushu++;
        cout << "第" << setw(4) << buzhoushu << " 步(" << setw(2)
            << n << "#: " << src << "-->" << dst << ")" << endl;
    }

    if (sel == '3' || sel == '4' || sel == '7' || sel == '8' || sel == '9')
        towers[dst - 'A'][top[dst - 'A']++] = towers[src - 'A'][--top[src - 'A']];//内部数组元素改变
    if (sel == '7') // 图形动画模式，但只调用第一次
        move_animate(1, src, dst, sel);
    if (sel == '3' || sel == '4' || sel == '8' || sel == '9')//横向数组
        henxiang(n, src, dst, sel, 1);
    if (sel == '4' || sel == '8' || sel == '9')//纵向数组
        zongxiang(n, src, dst, sel, 1);
    if (sel == '8' || sel == '9')  // 图形动画模式
        move_animate(n, src, dst, sel);
}

/***************************************************************************
   函数名称：横向移动总函数
   功    能：菜单3、4、8、9的横向输出
   输入参数：n:层数 src、dst:起始、目标柱 sel:菜单项
             loop:值为0的时候输出初始状态；值为1的时候输出初始状态（也可用bool型）
   返 回 值：空
   说    明：调用的print_horizontal_array()是核心横向数组打印
***************************************************************************/
void henxiang(int n, char src, char dst, char sel, int loop)
{
    if (sel == '3') {
        buzhoushu++;
        cout << "第" << setw(4) << buzhoushu << " 步(" << setw(2) << n
            << "#: " << src << "-->" << dst << ") ";
        print_horizontal_array();
        cout << endl;
    }
    else {
        if (sel == '4')
            cct_gotoxy(MenuItem4_Start_X, MenuItem4_Start_Y);
        if (sel == '8')
            cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y);
        if (sel == '9')
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y);
        if (loop == 0) {
            cout << "初始: ";
            print_horizontal_array();
        }
        else {
            if (sel == '4')
                delay_step(delay);
            else if ((buzhoushu <= 7 || delay != 0) && sel != '8')
                delay_step(delay);
            buzhoushu++;
            cout << "第" << setw(4) << buzhoushu << " 步(" << setw(2) << n
                << "#: " << src << "-->" << dst << ") ";
            print_horizontal_array();
        }
    }
}

/***************************************************************************
   函数名称：横向数组打印
   功    能：菜单3、4、8、9的横向数组打印
   输入参数：空
   返 回 值：空
   说    明：j = 0到j < top[i]打印数组数，之后打印空格，相较计算空格方式更加灵活
***************************************************************************/
void print_horizontal_array()
{
    for (int i = 0; i < 3; i++) {
        cout << " " << char('A' + i) << ":";
        for (int j = 0; j < top[i]; j++)
            cout << setw(2) << towers[i][j];
        for (int j = top[i]; j < MAX_LAYER; j++)
            cout << "  ";
    }
}

/***************************************************************************
   函数名称：纵向移动总函数
   功    能：菜单4、8、9的纵向数组打印
   输入参数：n:层数 src、dst:起始、目标柱 sel:菜单项
             loop:值为0的时候输出初始状态；值为1的时候输出初始状态（也可用bool型）
   返 回 值：空
   说    明：利用相对位置计算
***************************************************************************/
void zongxiang(int n, char src, char dst, char sel, int loop)
{
    int Start_X = 0, Start_Y = 0;
    if (sel == '4') {
        Start_X = MenuItem4_Start_X;
        Start_Y = MenuItem4_Start_Y;
    }
    else if (sel == '8') {
        Start_X = MenuItem8_Start_X;
        Start_Y = MenuItem8_Start_Y;
    }
    else {
        Start_X = MenuItem9_Start_X;
        Start_Y = MenuItem9_Start_Y;
    }

    if (loop == 0) {//初始化
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET, Start_Y + Underpan_A_Y_OFFSET);
        cout << "A";
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET + Underpan_Distance, Start_Y + Underpan_A_Y_OFFSET);
        cout << "B";
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET + 2 * Underpan_Distance, Start_Y + Underpan_A_Y_OFFSET);
        cout << "C";
        cct_gotoxy(Underpan_A_X_OFFSET + (src - 'A') * Underpan_Distance, Start_Y - top[src - 'A']);
        cout << "  ";
        for (int t = 0; t < 3; t++) {
            for (int i = 0, y = Start_Y + Underpan_A_Y_OFFSET - 1; i < top[t]; i++, y--) {
                cct_gotoxy(Start_X + Underpan_A_X_OFFSET + t * Underpan_Distance - 1, y - 1);
                cout << setw(2) << towers[t][i];
            }
        }
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET - 2, Start_Y + Underpan_A_Y_OFFSET - 1);
        for (int i = 0; i < 2 * Underpan_Distance + 5; i++)
            cout << "=";
    }
    else {
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET - 1 + (src - 'A') * Underpan_Distance,
            Start_Y + Underpan_A_Y_OFFSET - 2 - top[src - 'A']);
        cout << "  ";//除去原位置内容
        cct_gotoxy(Start_X + Underpan_A_X_OFFSET - 1 + (dst - 'A') * Underpan_Distance,
            Start_Y + Underpan_A_Y_OFFSET - 1 - top[dst - 'A']);
        cout << setw(2) << towers[dst - 'A'][top[dst - 'A'] - 1];// 输出新位置内容
    }
}

/***************************************************************************
   函数名称：打印柱子
   功    能：菜单5-9的打印柱子
   输入参数：n:层数 src、dst:起始、目标柱 sel:菜单项
   返 回 值：空
   说    明：利用相对位置计算
***************************************************************************/
void print_column(int n, char src, char dst, char sel)
{
    hdc_init(COLOR_BLACK, COLOR_WHITE, 8 * 120, 16 * 30);
    cct_gotoxy(Status_Line_X, Status_Line_Y);
    Sleep(100);

    for (int i = 0; i < 3; i++) {
        hdc_rectangle(HDC_Start_X + i * (23 * HDC_Base_Width + HDC_Underpan_Distance),
            HDC_Start_Y, 23 * HDC_Base_Width, HDC_Base_High, 0, true, 1, HDC_COLOR[MAX_LAYER + 1]);
        Sleep(HDC_Init_Delay);
    }
    for (int i = 0; i < 3; i++) {
        hdc_rectangle(HDC_Start_X + i * (23 * HDC_Base_Width + HDC_Underpan_Distance) + 11 * HDC_Base_Width,
            HDC_Start_Y - 12 * HDC_Base_High, HDC_Base_Width, 12 * HDC_Base_High, 0, true, 1, HDC_COLOR[MAX_LAYER + 1]);
        Sleep(HDC_Init_Delay);
    }
    if (sel != '5') {
        for (int i = n; i > 0; i--) {
            int column_x = HDC_Start_X + (src - 'A') * (23 * HDC_Base_Width + HDC_Underpan_Distance) + (23 * HDC_Base_Width) / 2;
            int disk_width = (2 * i + 1) * HDC_Base_Width;
            hdc_rectangle(column_x - disk_width / 2, HDC_Start_Y - (n + 1 - i) * HDC_Base_High,
                disk_width, HDC_Base_High, 0, true, 1, HDC_COLOR[i]);
            Sleep(HDC_Init_Delay);
        }
    }
}



/***************************************************************************
   函数名称：图形显示
   功    能：菜单8、9的图形显示
   输入参数：n:层数 src、dst:起始、目标柱 sel:菜单项
   返 回 值：空
   说    明：利用相对位置计算
***************************************************************************/
void move_animate(int n, char src, char dst, char sel)
{
    int start_x = HDC_Start_X + (src - 'A') * (23 * HDC_Base_Width + HDC_Underpan_Distance) + (23 * HDC_Base_Width) / 2 - (2 * n + 1) * HDC_Base_Width / 2;
    int start_y = HDC_Start_Y - (top[src - 'A'] + 1) * HDC_Base_High;

    while (start_y > HDC_Top_Y) {
        hdc_rectangle(start_x, start_y, (2 * n + 1) * HDC_Base_Width, HDC_Base_High, 0, true, 1, HDC_COLOR[0]);
        int column_center_x = HDC_Start_X + (src - 'A') * (23 * HDC_Base_Width + HDC_Underpan_Distance)
            + (23 * HDC_Base_Width) / 2;
        if (start_y + HDC_Base_High - HDC_Step_Y >= HDC_Start_Y - 12 * HDC_Base_High) {
            hdc_rectangle(column_center_x - HDC_Base_Width / 2,
                start_y + HDC_Base_High - HDC_Step_Y,
                HDC_Base_Width,
                HDC_Step_Y,
                0, true, 1, HDC_COLOR[MAX_LAYER + 1]);
        }
        start_y -= HDC_Step_Y;
        hdc_rectangle(start_x, start_y, (2 * n + 1) * HDC_Base_Width, HDC_Base_High, 0, true, 1, HDC_COLOR[n]);
        if ((buzhoushu <= 7) && (sel != '9'))
            delay_step(delay);
        if (sel == '9')
            Sleep(1);
    }

    int column_center_x = HDC_Start_X + (dst - 'A') * (23 * HDC_Base_Width + HDC_Underpan_Distance)
        + (23 * HDC_Base_Width) / 2;
    int target_x = column_center_x - ((2 * n + 1) * HDC_Base_Width) / 2;

    while (start_x != target_x) {
        int step, erase_x, draw_x;
        if (start_x < target_x)
            step = HDC_Step_X;
        else
            step = -HDC_Step_X;
        if (step > 0)
            erase_x = start_x;
        else
            erase_x = start_x + (2 * n + 1) * HDC_Base_Width - 1;
        hdc_rectangle(erase_x, start_y, HDC_Step_X, HDC_Base_High, 0, true, 1, HDC_COLOR[0]);
        start_x += step;
        if (step > 0)
            draw_x = start_x + (2 * n + 1) * HDC_Base_Width - HDC_Step_X;
        else
            draw_x = start_x;
        hdc_rectangle(draw_x, start_y, HDC_Step_X, HDC_Base_High, 0, true, 1, HDC_COLOR[n]);
        if ((buzhoushu <= 7) && (sel != '9'))
            delay_step(delay);
        if (sel == '9')
            Sleep(1);
    }

    int final_y = HDC_Start_Y - top[dst - 'A'] * HDC_Base_High;
    while (start_y < final_y) {
        hdc_rectangle(start_x, start_y, (2 * n + 2) * HDC_Base_Width, HDC_Base_High, 0, true, 1, HDC_COLOR[0]);
        int column_center_x = HDC_Start_X + (dst - 'A') * (23 * HDC_Base_Width + HDC_Underpan_Distance)
            + (23 * HDC_Base_Width) / 2;
        if (start_y >= HDC_Start_Y - 12 * HDC_Base_High) {
            hdc_rectangle(column_center_x - HDC_Base_Width / 2, start_y,
                HDC_Base_Width, HDC_Step_Y, 0, true, 1, HDC_COLOR[MAX_LAYER + 1]);
        }
        start_y += HDC_Step_Y;
        hdc_rectangle(start_x, start_y, (2 * n + 1) * HDC_Base_Width, HDC_Base_High, 0, true, 1, HDC_COLOR[n]);
        if ((buzhoushu <= 7) && (sel != '9'))
            delay_step(delay);
        if (sel == '9')
            Sleep(1);
    }
}
/***************************************************************************
   函数名称：游戏版功能
   功    能：菜单9的功能实现
   输入参数：n:层数 src、dst:起始、目标柱
   返 回 值：空
   说    明：处理多种情况
***************************************************************************/
void mode_9(int n, char src, char dst)
{
    bool game_end = false;
    const int INPUT_START_X = 61;//prompt长度

    while (!game_end) {
        char input_src = '\0', input_dst = '\0';

        cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 2);
        cout << "请输入移动的柱号(命令形式：AC=A顶端的盘子移动到C，Q=退出) ：";
        cct_gotoxy(MenuItem9_Start_X + INPUT_START_X, MenuItem9_Start_Y + 2);
        cout << "  ";
        cct_gotoxy(MenuItem9_Start_X + INPUT_START_X, MenuItem9_Start_Y + 2);

        int char_count = 0;
        // 读取字符直到回车
        while (true) {
            int ch = _getch();
            // 处理回车键
            if (ch == '\r')
                break;
            // 处理退格（demo中没处理，小优化）
            if (ch == '\b') {
                if (char_count > 0) {
                    char_count--;
                    cct_gotoxy(MenuItem9_Start_X + INPUT_START_X + char_count, MenuItem9_Start_Y + 2);
                    cout << " ";
                    cct_gotoxy(MenuItem9_Start_X + INPUT_START_X + char_count, MenuItem9_Start_Y + 2);
                    if (char_count == 0)
                        input_src = '\0';
                    else
                        input_dst = '\0';
                }
                continue;
            }
            // 小写转大写
            cout << char(ch);
            if (ch >= 'a' && ch <= 'c' || ch == 'q')
                ch -= 32;
            // 记录输入并显示
            if (ch >= 'A' && ch <= 'C' || ch == 'Q') {
                if (char_count == 0)
                    input_src = ch;
                else
                    input_dst = ch;
            }
            char_count++;
        }
        // 处理退出
        if ((input_src == 'Q' || input_src == 'q') && char_count == 1) {
            cct_gotoxy(Status_Line_X, Status_Line_Y);
            cout << "游戏中止!!!!!";
            return;
        }
        // 校验基本合法性
        if (input_src < 'A' || input_src > 'C' ||
            input_dst < 'A' || input_dst > 'C' ||
            input_src == input_dst || char_count != 2) {
            cct_gotoxy(MenuItem9_Start_X + INPUT_START_X, MenuItem9_Start_Y + 2);
            cout << "                                             "; // 清空字符
            continue;
        }

        // 源柱为空检查
        if (top[input_src - 'A'] == 0) {
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 3);
            cout << "源柱为空！";
            Sleep(1000);
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 3);
            cout << "                        "; // 清空提示
            cct_gotoxy(MenuItem9_Start_X + INPUT_START_X, MenuItem9_Start_Y + 2);
            cout << "  "; // 清空输入
            continue;
        }

        // 大盘压小盘检查
        if (top[input_dst - 'A'] > 0 && towers[input_src - 'A'][top[input_src - 'A'] - 1] > towers[input_dst - 'A'][top[input_dst - 'A'] - 1]) {
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 3);
            cout << "大盘压小盘，非法移动！";
            Sleep(1000);
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 3);
            cout << "                        ";
            cct_gotoxy(MenuItem9_Start_X + INPUT_START_X, MenuItem9_Start_Y + 2);
            cout << "  ";
            continue;
        }
        // 合法移动处理
        move(towers[input_src - 'A'][top[input_src - 'A'] - 1], input_src, input_dst, '9');

        // 胜利条件判断
        if (top[dst - 'A'] == n) {
            cct_gotoxy(MenuItem9_Start_X, MenuItem9_Start_Y + 2);
            //彩蛋（评估游戏实力）
            int min_step = 1;
            for (int i = 0; i < n; i++)
                min_step = 2 * min_step;
            min_step -= 1;
            if (buzhoushu == min_step)
                cout << "游戏结束!!!!!，完美移动（使用最短步数完成移动）";
            else if (buzhoushu <= (min_step + min_step / 5))
                cout << "游戏结束!!!!!，玩的还行（冗余步数不超过完美步数的1/5）";
            else
                cout << "游戏结束!!!!!，但移动的次数太多了（冗余步数超过完美步数的1/5）";
            game_end = true;
        }
    }
}