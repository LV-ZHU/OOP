/* 2452487 信安 胡中芃 */
#include <iostream>
#include <iomanip>  //用于debug信息打印时的格式
#include <fstream>
//不再允许加入任何头文件，特别是<Windows.h>/<Wingdi.h>，查到就是0分甚至是倒扣-20 ！！！
using namespace std;

#include "13-b7-bmp.h"

/***************************************************************************
  函数名称：构造函数
  功    能：
  输入参数：
  返 回 值：
  说    明：is_debug，用于查看调试信息，不需要实现（或者按自己的方式实现，对debug的输出不做要求）
            其余内容按需补充
***************************************************************************/
bitmap::bitmap(const char* const filename, const bool is_debug) : is_debug(is_debug), p(nullptr), q(nullptr)
{
    ifstream in(filename, ios::binary);
    if (!in) {
        cout << "打开文件[" << filename << "]失败." << endl;
        exit(-1);
    }

    const int file_and_info_length = 14 + 40;//BMP文件头(14)+信息头(40)
    unsigned char header[file_and_info_length] = { 0 };
    in.read((char*)(header), file_and_info_length);//强制转为char*，读取54个字节
    if (in.gcount() != file_and_info_length)
		exit(-1);

	bmp_header* t = (bmp_header*)header;//同demo，忽略42、4D（BM)等文件头信息是否正确，定义结构体指针只提取本题需要的信息
	true_picture_index = t->offset;
	width = t->width;
	height = t->height;
	bit_number = t->bit_count;
   
    int row_bits  = width * bit_number;// 行位数
	int row_bytes = ((row_bits + 31) / 32) * 4;//行字节数，向上取整到4的倍数防止一行不完整
	int color_count = 0;//调色板条目数

    //单色/16色/256色则存在调色板（更多色用hdc系列）；条目数=(true_picture_index -54) / 4
    if (bit_number <= 8 && true_picture_index > file_and_info_length) {
		color_count = (true_picture_index - file_and_info_length) / 4;//可以理解为color_count[N][4]N的数量
        if (color_count > 0) {
            p = new(nothrow) color_palette[color_count];
            if (p == nullptr) {
                cout << "调色板内存分配失败." << endl;
                exit(-1);
			}
            in.read((char*)(p), color_count * 4);
            if (in.gcount() != color_count * 4) {
                cout << "调色板读取不完整." << endl;
                delete[] p;
                p = nullptr;
                exit(-1);
            }
        }
    }

    in.seekg(true_picture_index, ios::beg); // 定位到像素数据起始位置
    if (!in) {
        cout << "定位像素数据失败." << endl;
        exit(-1);
    }

    int total_bytes = row_bytes * height; // 像素数据总字节数，注意行有填充
    q = new(nothrow) unsigned char[total_bytes];
    if (q == nullptr) {
        cout << "像素数据内存分配失败." << endl;
        exit(-1);
	}
    in.read((char*)(q),total_bytes);
    if (in.gcount() != total_bytes) {
        cout << "像素数据读取不完整." << endl;
        delete[] q;
        q = nullptr;
        exit(-1);
    }

    if (is_debug) {
        cout << " start=" << true_picture_index << endl
             << " width=" << width << endl
             << " height=" << height << endl
             << " bit_number=" << bit_number << endl
             << " row_bytes=" << row_bytes << endl
             << " palette=" << color_count << endl;
    }
}


/***************************************************************************
  函数名称：
  功    能：
  输入参数：
  返 回 值：
  说    明：按需补充
***************************************************************************/
bitmap::~bitmap()
{
    delete[] p;
    delete[] q;
}

//按需增加内容

int bitmap::show(const int top_left_x, const int top_left_y, const int angle, const bool is_mirror,
    void (*draw_point)(const int x, const int y, const unsigned char red, const unsigned char green, const unsigned char blue)) const
{
    if (!q || !draw_point) // 无像素读取/无绘制函数
        return -1;
    if (bit_number <= 8 && p == nullptr)//要调色板时没有
        return -1; 

    const int row_bytes = ((width * bit_number + 31) / 32) * 4;

    for (int y = 0; y < height; y++) { // 读取并显示所有像素（逻辑坐标：x 0..width-1, y 0..height-1，y=0为图像顶行）
        const unsigned char* row = q + (height - 1 - y) * row_bytes;// BMP实际存储是自底向上，因此要翻转行索引
        for (int x = 0; x < width; x++) {
            unsigned char red = 0, green = 0, blue = 0, color_index = 0;

            switch (bit_number)
            {
				case 1: //加括号限制作用域在case内
                {
					color_index = (row[x / 8] >> (7 - (x % 8))) & 0x01;
					//1位颜色下标是每字节从高位到低位依次是8个像素的颜色索引，然后右移7-(x%8)位取最低位
                    const color_palette& c = p[color_index];
                    blue = c.blue;
                    green = c.green;
                    red = c.red;
                    break;  
                }
                case 4: 
                {
                    color_index = (x % 2 == 0) ? ((row[x / 2] >> 4) & 0x0F) : (row[x / 2] & 0x0F);
					//4位颜色下标是每字节从高位到低位依次是2个像素的颜色索引，偶数像素右移4位取高4位，奇数像素取低4位
                    const color_palette& c = p[color_index];
                    blue = c.blue; 
                    green = c.green;
                    red = c.red;
                    break;   
                }
                case 8: 
                {
					const color_palette& c = p[row[x]];//8位颜色下标是每字节一个像素的颜色索引
                    blue = c.blue; 
                    green = c.green; 
                    red = c.red;
                    break;  
                }
                case 24: 
                {
                    const unsigned char* r = row + x * 3;
                    blue = r[0];
                    green = r[1];
                    red = r[2];
                    break;  
                }
                case 32: 
                {
                    const unsigned char* r = row + x * 4;
                    blue = r[0]; 
                    green = r[1];
                    red = r[2]; 
                    break;
                }
                default:
                    return -1;
            }         

            int mx = is_mirror ? (width - 1 - x) : x;//先以0度为基准镜像处理，y坐标不动，x坐标如果镜像则变为width-1-x  
            int rx = 0, ry = 0;//rotation x,y，旋转后x、y坐标，初始化为0，下面按角度逆时针旋转
            switch (angle)
            {
                case 0:
                    rx = mx;
                    ry = y;
                    break;
                case 90:
                    rx = y;
                    ry = width - 1 - mx;
                    break;
                case 180:
                    rx = width - 1 - mx;
                    ry = height - 1 - y;
                    break;
                case 270:
                    rx = height - 1 - y;
                    ry = mx;
                    break;
            }

            draw_point(top_left_x + rx, top_left_y + ry, red, green, blue);//加上起始top_left_x/y后用传入的画点函数
        }
    }

    return 0;
}