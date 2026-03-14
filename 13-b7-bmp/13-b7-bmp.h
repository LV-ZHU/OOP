/* 2452487 信安 胡中芃 */
#pragma once

//不允许加入任何头文件，特别是<Windows.h>/<Wingdi.h>，查到就是0分甚至是倒扣-20 ！！！

//关于BMP的微软官方文档：
// https://learn.microsoft.com/zh-cn/windows/win32/api/wingdi/ns-wingdi-bitmap

//自行查阅相关资料，了解下面这几个预编译命令的作用，看能否给你的作业带来方便！！！
//#pragma pack(show) //以警告信息的形式显示当前字节对齐的值
//#pragma pack(push) //将当前字节对齐值压入编译器栈的栈顶
//#pragma pack(push, 4) //将当前字节对齐值压入编译器栈的栈顶，然后再将4设置当前值
//#pragma pack(pop)  //将编译器栈栈顶的字节对齐值弹出并设置为当前值
//#pragma pack() //不带参数是恢复默认值


//允许定义其它需要的结构体（类）、常量、常变量等

#pragma pack(push,1) //防止字节对齐使读文件时读了错误的字节
struct bmp_header { //定义file_and_info_length结构体，大小为54字节，BMP前54字节非图片内容信息
	// 文件头：2+4+2+2+4=14字节
    unsigned short type;        // 对BMP而言应为BM
    unsigned int   size_of_bmp; // 文件大小,单位为字节
    unsigned short reserved1;   // 预留值必须为0
    unsigned short reserved2;   // 预留值必须为0
	unsigned int   offset;     // 文件头到实际数据的偏移字节数
	// 信息头：4+4+4+2+2+4+4+4+4+4+4=40字节
	unsigned int   size_of_information_header;  //信息头大小，BMP为40
    int            width;      //宽度 
    int            height;     //高度
	unsigned short planes;     //为目标设备说明面位数，其值总被设为1
	unsigned short bit_count; // 每个像素的位数
    unsigned int   compression;//压缩类型，本题不需要考虑有损
	unsigned int   size_of_image;//位头大小，单位为字节
	int            x_pelspermeter;//x方向分辨率，像素/米
	int            y_pelspermeter;//y方向分辨率，像素/米
	unsigned int   color_number;//颜色索引数，对于调色板为0表示默认值(2的n次方)
	unsigned int   color_important;//重要颜色数，对于调色板为0表示所有颜色都是重要的
}; 

struct color_palette {//系统内的调色板，4字节，分别存储蓝、绿、红、保留字节，前三位为RGB
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
};
#pragma pack(pop) //恢复设置

class bitmap {
private:
    const bool is_debug;
    /* 自行添加你需要的数据成员及成员函数，不限制数量 */
	int width; // 图像宽度
	int height; // 图像高度
	int true_picture_index; // BMP数据起始位置
	int bit_number; // 每像素位数
    color_palette* p;
    unsigned char* q;

public:
    /* 公有部分不允许添加任何内容 */
    bitmap(const char* const filename, const bool is_debug);
    ~bitmap();
    int show(const int top_left_x, const int top_left_y, const int angle, const bool is_mirror,
        void (*draw_point)(const int x, const int y, const unsigned char red, const unsigned char green, const unsigned char blue)) const;
};