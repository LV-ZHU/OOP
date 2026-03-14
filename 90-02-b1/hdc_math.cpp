/* 2452487 信安 胡中芃 */
#include "../include/cmd_hdc_tools.h" 
#include <iostream>
#include <math.h>
#include <conio.h>
using namespace std;
const double PI = 3.14159;

const int MAX_LEVEL = 10; // 雪花曲线最大阶数，阶数越高尤其在有延时的情况下越慢
const int size1 = 300;// 雪花大小，在cx、cy为400、400的情况下大小不要超过370，否则会超出窗口
const int cx = 400;// 雪花中心x坐标
const int cy = 400;// 雪花中心y坐标

static void curve(int x1, int y1, int x2, int y2, int level, int max_level)
{
    if (level == 0) {
        hdc_line(x1, y1, x2, y2, 1, (0, 0, 0)); // 最底层直接画线
        return;
    }
    // 三等分
    int xA = x1 + (x2 - x1) / 3;
    int yA = y1 + (y2 - y1) / 3;
    int xB = x1 + (x2 - x1) * 2 / 3;
    int yB = y1 + (y2 - y1) * 2 / 3;
    // 计算中间顶点
    double angle = atan2(y2 - y1, x2 - x1) - PI / 3; 
    double len = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) / 3;
    int xC = xA + (int)(len * cos(angle));
    int yC = yA + (int)(len * sin(angle));
    // 递归画四段
    curve(x1, y1, xA, yA, level - 1, max_level);
    curve(xA, yA, xC, yC, level - 1, max_level);
    curve(xC, yC, xB, yB, level - 1, max_level);
    curve(xB, yB, x2, y2, level - 1, max_level);
}

void hdc_draw_Mathematical_curve_2452487()
{
    hdc_cls(); 
    int level; // 分形阶数
    char ch;//选项
    while (1) {
        cout << "请输入雪花的阶数(0~" << MAX_LEVEL << ")：";//cct_showstr暂时有bug，故用cout代替
        cin >> level;
        if (level < 0 || level > MAX_LEVEL || cin.good() == 0) {
            cout << "输入有误，请重新输入！" << endl;
            cin.clear();
            cin.ignore(65536, '\n');
            hdc_cls();
        }
        else
            break;
    }

    while (1) {
        hdc_cls();
        cout << "<科赫雪花分形>" << endl;
        cout << "[按 a 增加阶数；按 d 减少阶数；按 c 退出]" << endl;
 
        int x1 = cx;
        int y1 = cy - size1;
        int x2 = cx + (int)(size1 * sqrt(3)/2);    
        int y2 = cy + (int)(size1 * 0.5);         
        int x3 = cx - (int)(size1 * sqrt(3)/2);   
        int y3 = cy + (int)(size1 * 0.5);         

        // 画三条边
        curve(x1, y1, x2, y2, level, level);
        curve(x2, y2, x3, y3, level, level);
        curve(x3, y3, x1, y1, level, level);

        int loop = 1;
        while (loop) {
            ch = _getch();
            switch (ch) {
                case 'a':
                    if (level < MAX_LEVEL) 
                        level++;
                    loop = 0;
                    break;
                case 'd':
                    if (level > 0) 
                        level--;
                    loop = 0;
                    break;
                case 'c':
                    return;
                default:
                    continue;
            }
        }
    }
}



