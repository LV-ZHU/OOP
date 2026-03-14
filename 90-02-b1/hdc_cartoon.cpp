/* 2452487 信安 胡中芃 */
#include <iostream>
#include <Windows.h>
#include <cmath>
#include "../include/cmd_hdc_tools.h"
//画的卡通形象是https://www.bilibili.com/video/BV1k6gXzwEXm/里的龙舌兰娘形象，原图为艺术作，采用了大量细节线条（包括颜色渐变、曲线等等），故本代码仅画出大致轮廓。

const int BACKGROUND_COLOR = RGB(115, 115, 115);   //图片背景色
const int COLOR_HAIR_BLUE = RGB(222, 236, 245);    // 头发的主体蓝色 
const int COLOR_HAIR_STRAND = RGB(150, 175, 200); // 头发丝的深蓝色 
const int COLOR_HAIRPIN = RGB(104, 13, 37);       // 发夹的颜色
const int COLOR_SKIN = RGB(255, 234, 228); // 脸部颜色
const int COLOR_CLOTHES_WHITE = RGB(255, 255, 255);// 衣服白色部分
const int COLOR_CLOTHES_ORANGE = RGB(240, 150, 80);// 衣服橙色部分

/***************************************************************************
   函数名称：
   功    能：画边缘六边形
   输入参数：RGB：填充色，raw_x0-raw_y5：六边形六个顶点的原始坐标
   返 回 值：
   说    明：通过将六边形拆分成四个三角形来进行填充绘制。
 ***************************************************************************/
static void hexagon(const int base_x, const int base_y, const int RGB,
    int raw_x0, int raw_y0, int raw_x1, int raw_y1,
    int raw_x2, int raw_y2, int raw_x3, int raw_y3,
    int raw_x4, int raw_y4, int raw_x5, int raw_y5)
{
    const int RGB2 = RGB(255, 255, 255); //边框色
    int fx0 = base_x + (raw_x0 - 275);
    int fy0 = base_y + (raw_y0 - 250);
    int fx1 = base_x + (raw_x1 - 275);
    int fy1 = base_y + (raw_y1 - 250);
    int fx2 = base_x + (raw_x2 - 275);
    int fy2 = base_y + (raw_y2 - 250);
    int fx3 = base_x + (raw_x3 - 275);
    int fy3 = base_y + (raw_y3 - 250);
    int fx4 = base_x + (raw_x4 - 275);
    int fy4 = base_y + (raw_y4 - 250);
    int fx5 = base_x + (raw_x5 - 275);
    int fy5 = base_y + (raw_y5 - 250);
    // 先用传入的RGB1颜色绘制填充色块
    hdc_triangle(fx0, fy0, fx1, fy1, fx2, fy2, true, 1, RGB);
    hdc_triangle(fx0, fy0, fx2, fy2, fx3, fy3, true, 1, RGB);
    hdc_triangle(fx0, fy0, fx3, fy3, fx4, fy4, true, 1, RGB);
    hdc_triangle(fx0, fy0, fx4, fy4, fx5, fy5, true, 1, RGB);
    // 再用RGB2颜色绘制宽度为2的白色边框
    hdc_line(fx0, fy0, fx1, fy1, 2, RGB2);
    hdc_line(fx1, fy1, fx2, fy2, 2, RGB2);
    hdc_line(fx2, fy2, fx3, fy3, 2, RGB2);
    hdc_line(fx3, fy3, fx4, fy4, 2, RGB2);
    hdc_line(fx4, fy4, fx5, fy5, 2, RGB2);
    hdc_line(fx5, fy5, fx0, fy0, 2, RGB2);
    Sleep(10);
}

/***************************************************************************
   函数名称：
   功    能：画所有的六边形
   输入参数：
   返 回 值：
   说    明：查看原图数值，传参
 ***************************************************************************/
static void draw_all_hexagons(const int base_x, const int base_y)
{  
    hexagon(base_x, base_y, RGB(0,0,0), 0,0,0,0,0,0,0,0,0,0,0,0);//第一个hexagon后会不明原因地调用cls，并非图形交叉导致，因此第一个置空，
    hexagon(base_x, base_y, RGB(242, 231, 217), 131, 136, 112, 147, 95, 184, 101, 205, 117, 192, 132, 161);
    hexagon(base_x, base_y, RGB(217, 218, 188), 95, 192, 77, 208, 72, 248, 81, 266, 95, 245, 102, 212);
    hexagon(base_x, base_y, RGB(224, 169, 104), 123, 210, 102, 220, 95, 256, 104, 275, 130, 263, 136, 229);
    hexagon(base_x, base_y, RGB(244, 231, 211), 84, 258, 72, 275, 73, 318, 87, 329, 102, 311, 100, 271);
    hexagon(base_x, base_y, RGB(236, 210, 130), 124, 277, 100, 287, 101, 319, 122, 347, 147, 335, 151, 298);
    hexagon(base_x, base_y, RGB(206, 206, 186), 101, 327, 83, 332, 87, 361, 112, 391, 129, 384, 127, 354);
    hexagon(base_x, base_y, RGB(224, 212, 131), 155, 345, 133, 355, 138, 385, 165, 408, 191, 400, 185, 366);
    hexagon(base_x, base_y, RGB(239, 230, 214), 89, 373, 80, 374, 86, 393, 106, 417, 118, 416, 112, 398);
    hexagon(base_x, base_y, RGB(232, 173, 102), 383, 123, 355, 122, 357, 152, 382, 177, 410, 173, 412, 149);
    hexagon(base_x, base_y, RGB(247, 246, 238), 462, 196, 458, 212, 462, 264, 474, 275, 480, 254, 471, 211);
    hexagon(base_x, base_y, RGB(245, 211, 138), 428, 216, 411, 237, 415, 276, 435, 290, 450, 267, 449, 231);
    hexagon(base_x, base_y, RGB(237, 227, 192), 352, 285, 317, 308, 313, 349, 343, 369, 380, 345, 382, 305);
    hexagon(base_x, base_y, RGB(247, 203, 123), 415, 288, 393, 309, 387, 348, 408, 362, 432, 337, 437, 301);
    hexagon(base_x, base_y, RGB(222, 224, 189), 479, 274, 463, 298, 457, 345, 469, 351, 483, 321, 486, 284);
    hexagon(base_x, base_y, RGB(182, 159, 103), 309, 351, 272, 372, 268, 399, 291, 414, 329, 403, 340, 371);
}

/***************************************************************************
   函数名称：
   功    能：画两朵花
   输入参数：
   返 回 值：
   说    明：边缘采用曲线近似
 ***************************************************************************/
static void draw_flower(const int base_x, const int base_y,
    int raw_center_x, int raw_center_y,
    int raw_v0_x, int raw_v0_y, int raw_v1_x, int raw_v1_y,
    int raw_v2_x, int raw_v2_y, int raw_v3_x, int raw_v3_y,
    int raw_v4_x, int raw_v4_y,
    int raw_f0_x, int raw_f0_y, int raw_f1_x, int raw_f1_y,
    int raw_f2_x, int raw_f2_y, int raw_f3_x, int raw_f3_y,
    int raw_f4_x, int raw_f4_y)
{
    const int COLOR_CENTER_INNER = RGB(255, 255, 220);
    const int COLOR_CENTER_OUTER = RGB(250, 233, 157);
    const int COLOR_OUTLINE = RGB(248, 210, 179);
    int fx_c = base_x + (raw_center_x - 275);
    int fy_c = base_y + (raw_center_y - 250);

    int outline_pts[10][2];
    outline_pts[0][0] = base_x + (raw_v0_x - 275); outline_pts[0][1] = base_y + (raw_v0_y - 250);
    outline_pts[1][0] = base_x + (raw_f0_x - 275); outline_pts[1][1] = base_y + (raw_f0_y - 250);
    outline_pts[2][0] = base_x + (raw_v1_x - 275); outline_pts[2][1] = base_y + (raw_v1_y - 250);
    outline_pts[3][0] = base_x + (raw_f1_x - 275); outline_pts[3][1] = base_y + (raw_f1_y - 250);
    outline_pts[4][0] = base_x + (raw_v2_x - 275); outline_pts[4][1] = base_y + (raw_v2_y - 250);
    outline_pts[5][0] = base_x + (raw_f2_x - 275); outline_pts[5][1] = base_y + (raw_f2_y - 250);
    outline_pts[6][0] = base_x + (raw_v3_x - 275); outline_pts[6][1] = base_y + (raw_v3_y - 250);
    outline_pts[7][0] = base_x + (raw_f3_x - 275); outline_pts[7][1] = base_y + (raw_f3_y - 250);
    outline_pts[8][0] = base_x + (raw_v4_x - 275); outline_pts[8][1] = base_y + (raw_v4_y - 250);
    outline_pts[9][0] = base_x + (raw_f4_x - 275); outline_pts[9][1] = base_y + (raw_f4_y - 250);
    const int y_min = base_y + (87 - 250);
    const double y_range = (base_y + (200 - 250)) - y_min;
    for (int i = 0; i < 10; ++i) {
        int* p_start = outline_pts[i];
        int* p_end = outline_pts[(i + 1) % 10];
        int cx = (p_start[0] + p_end[0]) / 2 + (int)((p_start[1] - p_end[1]) * 0.2);
        int cy = (p_start[1] + p_end[1]) / 2 + (int)((p_end[0] - p_start[0]) * 0.2);
        for (int k = 0; k < 20; ++k) {
            double r1 = k / 20.0, r2 = (k + 1) / 20.0;
            double x1 = (1 - r1) * (1 - r1) * p_start[0] + 2 * (1 - r1) * r1 * cx + r1 * r1 * p_end[0];
            double y1 = (1 - r1) * (1 - r1) * p_start[1] + 2 * (1 - r1) * r1 * cy + r1 * r1 * p_end[1];
            double x2 = (1 - r2) * (1 - r2) * p_start[0] + 2 * (1 - r2) * r2 * cx + r2 * r2 * p_end[0];
            double y2 = (1 - r2) * (1 - r2) * p_start[1] + 2 * (1 - r2) * r2 * cy + r2 * r2 * p_end[1];

            double avg_y = (fy_c + y1 + y2) / 3.0;
            double t = (avg_y - y_min) / y_range; 
            if (t < 0)
                t = 0; 
            if (t > 1)
                t = 1;
            int r = (int)(242 * (1 - t) + 252 * t);
            int g = (int)(199 * (1 - t) + 206 * t);
            int b = (int)(197 * (1 - t) + 135 * t);
            hdc_triangle(fx_c, fy_c, (int)x1, (int)y1, (int)x2, (int)y2, true, 1, RGB(r, g, b));
        }
    }   
    for (int i = 0; i < 5; ++i) {
        int* current_fold = outline_pts[i * 2 + 1];
        int* next_fold = outline_pts[((i + 1) * 2 + 1) % 10];
        hdc_triangle(fx_c, fy_c, fx_c + (current_fold[0] - fx_c) / 3, fy_c + (current_fold[1] - fy_c) / 3, fx_c + (next_fold[0] - fx_c) / 3, fy_c + (next_fold[1] - fy_c) / 3, true, 1, COLOR_CENTER_OUTER);
        hdc_triangle(fx_c, fy_c, fx_c + (current_fold[0] - fx_c) / 6, fy_c + (current_fold[1] - fy_c) / 6, fx_c + (next_fold[0] - fx_c) / 6, fy_c + (next_fold[1] - fy_c) / 6, true, 1, COLOR_CENTER_INNER);
    }
    
    for (int i = 0; i < 10; ++i) { 
        int* p_start = outline_pts[i];
        int* p_end = outline_pts[(i + 1) % 10];
        int cx = (p_start[0] + p_end[0]) / 2 + (int)((p_start[1] - p_end[1]) * 0.2);
        int cy = (p_start[1] + p_end[1]) / 2 + (int)((p_end[0] - p_start[0]) * 0.2);
        for (int k = 0; k < 20; ++k) {
            double r1 = k / 20.0, r2 = (k + 1) / 20.0;
            double x1 = (1 - r1) * (1 - r1) * p_start[0] + 2 * (1 - r1) * r1 * cx + r1 * r1 * p_end[0];
            double y1 = (1 - r1) * (1 - r1) * p_start[1] + 2 * (1 - r1) * r1 * cy + r1 * r1 * p_end[1];
            double x2 = (1 - r2) * (1 - r2) * p_start[0] + 2 * (1 - r2) * r2 * cx + r2 * r2 * p_end[0];
            double y2 = (1 - r2) * (1 - r2) * p_start[1] + 2 * (1 - r2) * r2 * cy + r2 * r2 * p_end[1];
            hdc_line((int)x1, (int)y1, (int)x2, (int)y2, 2, COLOR_OUTLINE);
        }
    } 
}

/***************************************************************************
   函数名称：
   功    能：画火焰
   输入参数：
   返 回 值：
   说    明：用弧近似火焰效果
 ***************************************************************************/
static void draw_flame(const int base_x, const int base_y)
{
	const int RGB = RGB(238, 115, 31);
    hdc_arc(base_x - 5, base_y + 5, 208, 120, 240, 18, RGB);
}

/***************************************************************************
   函数名称：
   功    能：画头饰
   输入参数：
   返 回 值：
   说    明：两段弧，两段弓
 ***************************************************************************/
static void draw_headdress(const int base_x, const int base_y)
{
    const int RGB = RGB(207, 145, 111);
    hdc_sector(base_x - 13, base_y - 103, 100, -50, 50, true, 5, RGB);
    Sleep(50);
    hdc_arc(base_x - 13, base_y - 103, 100, -50, 50, 5, RGB(0,0,0));
    Sleep(50);
    hdc_arc(base_x - 13, base_y - 103, 70, -50, 50, 5, RGB(155, 101, 83));
    Sleep(50);
    hdc_line(base_x - 69, base_y - 144, base_x - 90, base_y - 170, 5, RGB(0, 0, 0));//左弓
    hdc_line(base_x + 40, base_y - 144, base_x + 61, base_y - 170, 5, RGB(0, 0, 0));//右弓
}

/***************************************************************************
   函数名称：
   功    能：头部分，内容见下
   输入参数：
   返 回 值：
   说    明：
 ***************************************************************************/
static void draw_head(const int base_x, const int base_y) 
{   
    hdc_triangle(base_x + 40, base_y - 143, base_x - 14, base_y - 117, base_x - 69, base_y - 144, true, 10, COLOR_HAIR_BLUE);
    hdc_triangle(base_x - 106, base_y - 20, base_x - 69, base_y - 144, base_x + 40, base_y - 143, true, 1, COLOR_HAIR_BLUE);
    hdc_triangle(base_x - 106, base_y - 20, base_x + 40, base_y - 143, base_x + 54, base_y - 77, true, 1, COLOR_HAIR_BLUE);
    hdc_triangle(base_x - 106, base_y - 20, base_x + 54, base_y - 77, base_x + 53, base_y + 5, true, 1, COLOR_HAIR_BLUE);
    hdc_triangle(base_x - 106, base_y - 20, base_x + 53, base_y + 5, base_x + 23, base_y + 5, true, 1, COLOR_HAIR_BLUE);
    hdc_triangle(base_x - 106, base_y - 20, base_x + 23, base_y + 5, base_x - 41, base_y + 10, true, 1, COLOR_HAIR_BLUE);

    hdc_line(base_x - 86, base_y - 20, base_x - 46, base_y - 124, 20, COLOR_HAIR_STRAND);//头发丝1
    hdc_line(base_x - 66, base_y - 20, base_x - 46, base_y - 127, 20, COLOR_HAIR_STRAND);//头发丝2
    hdc_line(base_x - 23, base_y -55, base_x - 2, base_y - 105, 20, COLOR_HAIR_STRAND);//头发丝3
    hdc_line(base_x , base_y -50, base_x+3 , base_y - 97, 20, COLOR_HAIR_STRAND);//头发丝4
    hdc_line(base_x + 37, base_y -58, base_x +26, base_y - 105, 20, COLOR_HAIR_STRAND);//头发丝5

    hdc_line(base_x - 106, base_y - 20, base_x - 69, base_y - 144, 5, RGB(0, 0, 0));//左边线
    hdc_line(base_x - 106, base_y - 20, base_x - 41, base_y + 10, 5, RGB(0, 0, 0));//左边线
    hdc_line(base_x + 40, base_y - 143, base_x + 54, base_y - 77, 5, RGB(0, 0, 0));//右边线
    hdc_rectangle(base_x + 14, base_y - 80, 14, 30, -80, true, 2, COLOR_HAIRPIN);//横发夹
    hdc_rectangle(base_x + 39, base_y - 100, 30, 14, 100, true, 2, COLOR_HAIRPIN);//竖发夹
    hdc_line(base_x + 54, base_y - 77, base_x + 53, base_y + 5, 5, RGB(0, 0, 0));//右边线
    hdc_line(base_x + 53, base_y + 5, base_x + 23, base_y + 5, 5, RGB(0, 0, 0));//右边线
    hdc_rectangle(base_x - 60, base_y - 45, 85, 40, 0, true, 1, COLOR_SKIN);
    hdc_triangle(base_x - 41, base_y + 10, base_x + 23, base_y + 5, base_x - 9, base_y - 10, true, 1, COLOR_SKIN);// 使用一个三角形来填充下巴区域，连接到嘴巴下方
    hdc_line(base_x - 59, base_y - 35, base_x - 29, base_y - 35, 8, RGB(0, 0, 0));//左眼
    hdc_line(base_x - 9, base_y - 35, base_x + 21, base_y - 35, 8, RGB(0, 0, 0));//右眼
    hdc_line(base_x - 29, base_y - 15, base_x - 9, base_y - 15, 8, RGB(0, 0, 0));//嘴巴
}

/***************************************************************************
   函数名称：
   功    能：身体部分，内容见下
   输入参数：
   返 回 值：
   说    明：
 ***************************************************************************/
static void draw_body(const int base_x, const int base_y) 
{
    // 填充腿部皮肤
    hdc_triangle(base_x - 41, base_y + 94, base_x - 40, base_y + 200, base_x - 14, base_y + 210, true, 1, COLOR_SKIN);
    hdc_triangle(base_x - 6, base_y + 124, base_x - 14, base_y + 210, base_x - 41, base_y + 94, true, 1, COLOR_SKIN);
    hdc_triangle(base_x -6, base_y + 124, base_x + 21, base_y + 210, base_x + 27, base_y + 97, true, 1, COLOR_SKIN);
    // 填充白色连衣裙
    hdc_triangle(base_x - 41, base_y + 10, base_x - 41, base_y + 94, base_x - 6, base_y + 124, true, 1, COLOR_CLOTHES_WHITE);
    hdc_triangle(base_x - 41, base_y + 10, base_x - 6, base_y + 124, base_x + 27, base_y + 97, true, 1, COLOR_CLOTHES_WHITE);
    hdc_triangle(base_x - 41, base_y + 10, base_x + 27, base_y + 97, base_x + 23, base_y + 5, true, 1, COLOR_CLOTHES_WHITE);
    // 填充手臂皮肤和橙色袖子
    hdc_triangle(base_x - 41, base_y + 10, base_x - 68, base_y + 88, base_x - 41, base_y + 94, true, 1, COLOR_CLOTHES_ORANGE); // 左袖
    hdc_triangle(base_x + 23, base_y + 5, base_x + 55, base_y + 70, base_x + 27, base_y + 97, true, 1, COLOR_CLOTHES_ORANGE); // 右袖
    hdc_line(base_x - 41, base_y + 10, base_x - 68, base_y + 88, 5, RGB(0, 0, 0));//左臂
    hdc_line(base_x - 68, base_y + 88, base_x - 41, base_y + 94, 5, RGB(0, 0, 0));//左手 
    hdc_line(base_x - 41, base_y + 94, base_x - 40, base_y + 200, 5, RGB(0, 0, 0));//左腿左 
    hdc_line(base_x - 6, base_y + 124, base_x - 14, base_y + 210, 5, RGB(0, 0, 0));//左腿右 
    hdc_line(base_x - 6, base_y + 124, base_x + 21, base_y + 210, 5, RGB(0, 0, 0));//右腿1 
    hdc_line(base_x + 23, base_y + 5, base_x + 55, base_y + 70, 5, RGB(0, 0, 0));//右腿2
    hdc_line(base_x + 55, base_y + 70, base_x + 27, base_y + 97, 5, RGB(0, 0, 0)); //右腿3
    hdc_line(base_x + 27, base_y + 97, base_x + 24, base_y + 210, 5, RGB(0, 0, 0));//右腿4
}

void hdc_draw_cartoon_2452487(const int base_x, const int base_y)
{
    hdc_cls();
   
    hdc_rectangle(base_x - 275, base_y - 250, 550, 500, 0, true, 1, BACKGROUND_COLOR);
    Sleep(100);//确保背景正常显示
    draw_all_hexagons(base_x, base_y);
    draw_flower(base_x, base_y, 448, 160,
        455, 87, 400, 113, 411, 190, 493, 200, 514, 129, 
        434, 117, 419, 154, 441, 188, 497, 164, 477, 125); 
    draw_flower(base_x, base_y, 394, 231,
        375, 194, 353, 237, 389, 273, 435, 250, 425, 198, 
        368, 218, 375, 255, 411, 257, 424, 227, 401, 199); 
    draw_flame(base_x, base_y); 
    draw_headdress(base_x, base_y);
    draw_head(base_x, base_y);
    draw_body(base_x, base_y);

}