/* 2452487 信安 胡中芃 */
#pragma once
#include <string>
#include <sstream>
#include <istream>
using namespace std;

#define MAX_COMPARE_BYTE 65536

class txt_compare {
private:
    string filename1;
    string filename2;
    string trim_type;
    string display_type;
    int line_skip;
    int line_offset;
    int max_diffnum;
    int max_linenum;
    bool ignore_blank;
    bool not_ignore_linefeed;
    bool debug;
    ostringstream output_stream;
    void print_diff_line(const char* s, const string& str, bool eof, int minlen, const char* s_other);

    bool use_mem = false;
    string mem1, mem2;

public:
    txt_compare(const string& filename1, const string& filename2,
        const string& trim_type, const string& display_type,
        int line_skip, int line_offset, int max_diffnum, int max_linenum,
        bool ignore_blank, bool not_ignore_linefeed, bool debug);

    txt_compare(istringstream& in1, istringstream& in2,
        const string& trim_type, const string& display_type,
        int line_skip, int line_offset, int max_diffnum, int max_linenum,
        bool ignore_blank, bool not_ignore_linefeed, bool debug);

    int compare(bool is_print_diff);

    // 核心：compare 的原逻辑搬到这里；compare() 只负责准备输入流（文件 or 内存）
    int compare_core(istream& in1, istream& in2, istream* bin1, istream* bin2, bool silent);

    void result();
};