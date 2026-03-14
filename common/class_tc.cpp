/* 2452487 信安 胡中芃 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/class_tc.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <sstream>
#include "../include/cmd_console_tools.h"
#include "../include/line_process_tools.h"
using namespace std;

//txt_compare --file1 1.txt --file2 6.txt --display normal --lineoffset -1 --ignore_blank，
//新版处理了trim的动态更新问题，旧版未处理；新版没处理65536的检测，旧版处理了
//txt_compare --file1 1.txt --file2 2.txt --trim none --lineskip 0 --lineoffset -5 --max_diff 0 --max_line 0 --display detailed --ignore_blank --not_ignore_linefeed > a.txt

/***************************************************************************
  函数名称：
  功    能：构造函数，把各变量用初始化表初始化
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
txt_compare::txt_compare(const string& filename1, const string& filename2,
    const string& trim_type, const string& display_type,
    int line_skip, int line_offset, int max_diffnum, int max_linenum,
    bool ignore_blank, bool not_ignore_linefeed, bool debug) : filename1(filename1), filename2(filename2), trim_type(trim_type),
    display_type(display_type), line_skip(line_skip), line_offset(line_offset),
    max_diffnum(max_diffnum), max_linenum(max_linenum),
    ignore_blank(ignore_blank), not_ignore_linefeed(not_ignore_linefeed),
    debug(debug)
{
}

/***************************************************************************
  函数名称：
  功    能：重载构造函数，支持两个istringstream做比对
  说    明：
***************************************************************************/
txt_compare::txt_compare(istringstream& in1, istringstream& in2,
    const string& trim_type, const string& display_type,
    int line_skip, int line_offset, int max_diffnum, int max_linenum,
    bool ignore_blank, bool not_ignore_linefeed, bool debug)
    : filename1(""), filename2(""), trim_type(trim_type),
    display_type(display_type), line_skip(line_skip), line_offset(line_offset),
    max_diffnum(max_diffnum), max_linenum(max_linenum),
    ignore_blank(ignore_blank), not_ignore_linefeed(not_ignore_linefeed),
    debug(debug)
{
    use_mem = true;
    mem1 = in1.str();
    mem2 = in2.str();
}

/***************************************************************************
  函数名称：
  功    能：
  返 回 值：int-不一致的行数；0表示一致
  说    明：
***************************************************************************/
int txt_compare::compare(bool is_print_diff)
{
	bool silent = !(is_print_diff);//用silent表示是否静默，不打印差异行

    if (use_mem) {
        istringstream in1(mem1);
        istringstream in2(mem2);
        istringstream bin1(mem1);
        istringstream bin2(mem2);
        return compare_core(in1, in2, &bin1, &bin2, silent);
    }

    ifstream in1, in2;
    if (not_ignore_linefeed) {//windows/linux换行符不等价，则以二进制方式打开
        in1.open(filename1, ios::in | ios::binary);
        in2.open(filename2, ios::in | ios::binary);
    }
    else {//普通文本方式直接打开（比较时忽略换行符差异）
        in1.open(filename1, ios::in);
        in2.open(filename2, ios::in);
    }
    if (!in1.is_open()) {
        if (!silent) {
            output_stream << "第1个文件[" << filename1 << "]无法打开." << endl;
        }
        return 1;
    }
    if (!in2.is_open()) {
        if (!silent) {
            output_stream << "第2个文件[" << filename2 << "]无法打开." << endl;
        }
        in1.close();
        return 1;
    }

    // 额外打开两个二进制流，用于判断行结束符并用于宽度计算与显示（不改变主比较流的打开模式）
    ifstream bin1(filename1, ios::in | ios::binary);
    ifstream bin2(filename2, ios::in | ios::binary);
    istream* pbin1 = nullptr;
    istream* pbin2 = nullptr;
    if (bin1.is_open() && bin2.is_open()) {
        pbin1 = &bin1;
        pbin2 = &bin2;
    }

    int ret = compare_core(in1, in2, pbin1, pbin2, silent);

    if (bin1.is_open())
        bin1.close();
    if (bin2.is_open())
        bin2.close();
    in1.close();
    in2.close();

    return ret;
}

/***************************************************************************
  函数名称：
  功    能：
  返 回 值：
  说    明：原来tc作业里核心的判断流程放到这个函数来，让txt_comapare::compare()准备输入流（文件or内存）
***************************************************************************/
int txt_compare::compare_core(istream& in1, istream& in2, istream* bin1, istream* bin2, bool silent)
{
    bool bin_open = (bin1 != nullptr && bin2 != nullptr);

    int line1_num = 0;//file1当前行号
    int line2_num = 0;//file2当前行号
    int diff_count = 0;//差异行数
    int total_count = 0;//当前总比较行数
    int line_maxlen = 0;//最大行宽，用于display detailed时的分隔线打印

    // 先用二进制流计算最大行宽（确保能区分 CRLF/LF）
    if (bin_open) {
        string raw1, raw2;
        int temp_diff = 0;
        int temp_count = 0;
        bin1->clear();
        bin2->clear();
        bin1->seekg(0, ios::beg);
        bin2->seekg(0, ios::beg);

        while (!bin1->eof() && !bin2->eof()) {
            if (max_diffnum > 0 && temp_diff >= max_diffnum)
                break;
            if (max_linenum > 0 && temp_count >= max_linenum)
                break;
            bool ok1 = static_cast<bool>(getline(*bin1, raw1));
            bool ok2 = static_cast<bool>(getline(*bin2, raw2));
            line1_num++;
            line2_num++;
            bool had_delim1 = ok1 && (!bin1->eof());
            bool had_delim2 = ok2 && (!bin2->eof());
            int lineEndLen1 = 0;
            if (ok1) {
                if (had_delim1) {
                    if (!raw1.empty() && raw1.back() == '\r')
                        lineEndLen1 = 2;
                    else
                        lineEndLen1 = 1;
                }
                else
                    lineEndLen1 = 0;
            }

            int lineEndLen2 = 0;
            if (ok2) {
                if (had_delim2) {
                    if (!raw2.empty() && raw2.back() == '\r')
                        lineEndLen2 = 2;
                    else
                        lineEndLen2 = 1;
                }
                else
                    lineEndLen2 = 0;
            }
            string content1 = raw1;
            string content1_no_cr = content1;
            if (!content1_no_cr.empty() && lineEndLen1 == 2 && content1_no_cr.back() == '\r')
                content1_no_cr.pop_back();
            string ts1 = content1_no_cr;
            trim_accordingly(ts1, trim_type);
            int len1 = static_cast<int>(ts1.length()); // 不包含行尾 CR/LF

            string content2 = raw2;
            string content2_no_cr = content2;
            if (!content2_no_cr.empty() && lineEndLen2 == 2 && content2_no_cr.back() == '\r')
                content2_no_cr.pop_back();
            string ts2 = content2_no_cr;
            trim_accordingly(ts2, trim_type);
            int len2 = static_cast<int>(ts2.length()); // 不包含行尾 CR/LF
            // 超长检查
            if (len1 > MAX_COMPARE_BYTE || len2 > MAX_COMPARE_BYTE) {
                if (!silent) {
                    if (len1 > MAX_COMPARE_BYTE) {
                        cout << "文件[" << filename1 << "]的第(" << line1_num << ")行不符合要求，超过最大长度[" << MAX_COMPARE_BYTE << "]." << endl;
                    }
                    if (len2 > MAX_COMPARE_BYTE) {
                        cout << "文件[" << filename2 << "]的第(" << line2_num << ")行不符合要求，超过最大长度[" << MAX_COMPARE_BYTE << "]." << endl;
                    }
                }

                return 1;
            }

            int current_max = (len1 > len2) ? len1 : len2;
            if (current_max > line_maxlen)
                line_maxlen = current_max;
            if (ts1 != ts2 || had_delim1 != had_delim2)
                temp_diff++;
            temp_count++;
        }
        bin1->clear();
        bin2->clear();
        bin1->seekg(0, ios::beg);
        bin2->seekg(0, ios::beg);
        line1_num = 0;
        line2_num = 0;
    }
    // 分隔行===的宽度
    int width = (line_maxlen / 10 + 1) * 10 + 8 + 2;
    if (display_type == "detailed" && width < 80) {
        width = 80;
    }
    if (display_type != "none") {
        if (!silent) {
            output_stream << "比较结果输出：" << endl;
            output_stream << string(width, '=') << endl;
        }
    }
    bool is_offset_done = false;
    bool is_skipped_done = false;

    // 正式比较主循环，所有读取与之前保持一致
    string str1, str2, raw1, raw2;
    while (!in1.eof() && !in2.eof()) {
        if (max_diffnum > 0 && diff_count >= max_diffnum)
            break;
        if (max_linenum > 0 && total_count >= max_linenum)
            break;

        bool ok_main1 = static_cast<bool>(getline(in1, str1));
        bool ok_main2 = static_cast<bool>(getline(in2, str2));
        (void)ok_main1;
        (void)ok_main2;
        line1_num++;
        line2_num++;

        // 读取 bin raw（如果 bin 可用），注意检查返回值并在失败时清空 raw，避免用旧值
        if (bin_open) {
            bool ok_bin1 = static_cast<bool>(getline(*bin1, raw1));
            if (!ok_bin1) 
                raw1.clear();
            bool ok_bin2 = static_cast<bool>(getline(*bin2, raw2));
            if (!ok_bin2) 
                raw2.clear();
        }
        else {
            raw1 = str1;
            raw2 = str2;
        }

        string s1 = str1;
        string s2 = str2;
        trim_accordingly(s1, trim_type);
        trim_accordingly(s2, trim_type);

        // ignore_blank
        if (ignore_blank) {
            while (s1.empty() && !in1.eof()) {
                getline(in1, str1);
                if (bin_open) {
                    if (!getline(*bin1, raw1))
                        raw1.clear();
                }
                s1 = str1;
                trim_accordingly(s1, trim_type);
                line1_num++;
            }
            while (s2.empty() && !in2.eof()) {
                getline(in2, str2);
                if (bin_open) {
                    if (!getline(*bin2, raw2)) 
                        raw2.clear();
                }
                s2 = str2;
                trim_accordingly(s2, trim_type);
                line2_num++;
            }
        }

        // line_offset（只执行一次）
        if (line_offset != 0 && !is_offset_done) {
            if (line_offset < 0) {
                int need = -line_offset;
                int skipped = 0;
                string raw;
                while (skipped < need) {
                    if (!getline(in1, raw))
                        break;
                    if (bin_open) {
                        if (!getline(*bin1, raw1))
                            raw1.clear();
                    }
                    line1_num++;
                    string t = raw;
                    trim_accordingly(t, trim_type);
                    if (!ignore_blank || !t.empty())
                        skipped++;
                }
                str1 = raw;
            }
            else {
                int need = line_offset;
                int skipped = 0;
                string raw;
                while (skipped < need) {
                    if (!getline(in2, raw))
                        break;
                    if (bin_open) {
                        if (!getline(*bin2, raw2))
                            raw2.clear();
                    }
                    line2_num++;
                    string t = raw;
                    trim_accordingly(t, trim_type);
                    if (!ignore_blank || !t.empty())
                        skipped++;
                }
                str2 = raw;
            }
            is_offset_done = true;
            s1 = str1;
            s2 = str2;
            trim_accordingly(s1, trim_type);
            trim_accordingly(s2, trim_type);
        }

        // line_skip（只执行一次）
        if (line_skip > 0 && !is_skipped_done) {
            for (int k = 0; k < line_skip; ++k) {
                string raw_1;
                while (true) {
                    if (!getline(in1, raw_1)) {
                        raw_1.clear();
                        break;
                    }
                    if (bin_open) {
                        if (!getline(*bin1, raw1)) 
                            raw1.clear();
                    }
                    line1_num++;
                    string t1 = raw_1;
                    trim_accordingly(t1, trim_type);
                    if (!ignore_blank || !t1.empty())
                        break;
                }
                string raw_2;
                while (true) {
                    if (!getline(in2, raw_2)) {
                        raw_2.clear();
                        break;
                    }
                    if (bin_open) {
                        if (!getline(*bin2, raw2))
                            raw2.clear();
                    }
                    line2_num++;
                    string t2 = raw_2;
                    trim_accordingly(t2, trim_type);
                    if (!ignore_blank || !t2.empty())
                        break;
                }
                str1 = raw_1;
                str2 = raw_2;
            }
            is_skipped_done = true;
            s1 = str1;
            s2 = str2;
            trim_accordingly(s1, trim_type);
            trim_accordingly(s2, trim_type);
        }
        total_count++;

        bool is_same = false;
        bool endline_only_diff = false;
        (void)endline_only_diff;

        if (s1 == s2) {
            bool equal = true;
            if (ignore_blank) {
                if (in1.eof() && !in2.eof()) {
                    streampos pos = in2.tellg();
                    streampos binPos = (bin_open ? bin2->tellg() : streampos(-1));
                    string tmp;
                    while (getline(in2, tmp)) {
                        string t = tmp;
                        trim_accordingly(t, trim_type);
                        if (!t.empty()) { 
                            equal = false; 
                            break; 
                        }
                    }
                    in2.clear();
                    in2.seekg(pos);
                    if (bin_open) { 
                        bin2->clear(); 
                        bin2->seekg(binPos);
                    }
                }
                else if (in2.eof() && !in1.eof()) {
                    streampos pos = in1.tellg();
                    streampos binPos = (bin_open ? bin1->tellg() : streampos(-1));
                    string tmp;
                    while (getline(in1, tmp)) {
                        string t = tmp;
                        trim_accordingly(t, trim_type);
                        if (!t.empty()) { 
                            equal = false; 
                            break;
                        }
                    }
                    in1.clear();
                    in1.seekg(pos);
                    if (bin_open) {
                        bin1->clear(); 
                        bin1->seekg(binPos); 
                    }
                }
            }

            if (equal) {
                // 新增：即使内容相同，也要检测是否仅行结束符不同（仅当有二进制原始数据可用时）
                if (bin_open) {
                    string c1 = raw1;
                    string c2 = raw2;
                    bool hasCR1 = (!c1.empty() && c1.back() == '\r');
                    bool hasCR2 = (!c2.empty() && c2.back() == '\r');

                    // 去掉可能的 CR，再按 trim_type 比较
                    string t1 = c1;
                    string t2 = c2;
                    if (hasCR1) t1.pop_back();
                    if (hasCR2) t2.pop_back();
                    trim_accordingly(t1, trim_type);
                    trim_accordingly(t2, trim_type);

                    if (t1 == t2 && hasCR1 != hasCR2) {
                        // 仅行结束符不同，视为差异，让下面差异分支处理并输出“行结束符不同”
                        endline_only_diff = true;
                        is_same = false;
                    }
                    else {
                        is_same = true;
                    }
                }
                else {
                    // 没有二进制辅助流，按原逻辑认为相同
                    is_same = true;
                }
            }
        }

        if (!is_same) {
            diff_count++;
            if (display_type == "none") {
                if (!silent) {
                    output_stream << "文件不同.\n";
                }
                return diff_count;
            }

            if (!silent) {
                // 计算第一个不同字符位置
                output_stream << "第[" << line1_num << " / " << line2_num << "]行 - ";
            }
            int len1 = static_cast<int>(s1.length());
            int len2 = static_cast<int>(s2.length());
            int minlen = (len1 < len2) ? len1 : len2;
            int i = 0;
            for (i = 0; i < minlen; i++) {
                if (s1[i] != s2[i]) {
                    if (!silent) {
                        output_stream << "第[" << i << "]个字符开始有差异" << endl;
                    }
                    break;
                }
            }

            // 当前已比较到 minlen（前缀相同或达到短行尾），需要区分：行结束符差异 / 文件结束 / 有多余字符
            if (i == minlen) {
                if (bin_open) {
                    // 取二进制读取的原始行，去掉可能的 '\r' 后按 trim_type 比较（但不修改 raw1/raw2）
                    string content1 = raw1;
                    string content1_no_cr = content1;
                    if (!content1_no_cr.empty() && content1_no_cr.back() == '\r')
                        content1_no_cr.pop_back();
                    string content2 = raw2;
                    string content2_no_cr = content2;
                    if (!content2_no_cr.empty() && content2_no_cr.back() == '\r')
                        content2_no_cr.pop_back();
                    string t1 = content1_no_cr;
                    string t2 = content2_no_cr;
                    trim_accordingly(t1, trim_type);
                    trim_accordingly(t2, trim_type);

                    if (t1 == t2) {
                        // 仅当经 trim 后内容相同且只有行尾字节差异时，判定为“行结束符不同”
                        bool hasCR1 = !raw1.empty() && raw1.back() == '\r';
                        bool hasCR2 = !raw2.empty() && raw2.back() == '\r';
                        if (hasCR1 != hasCR2) {
                            if (!silent) 
                                output_stream << "行结束符不同" << endl;
                        }
                        else if (in1.eof() != in2.eof()) {
                            if (!silent) {
                                if (in1.eof())
                                    output_stream << "文件1已结束/文件2仍有内容" << endl;
                                else
                                    output_stream << "文件2已结束/文件1仍有内容" << endl;
                            }
                        }
                        else if (len1 != len2) {
                            if (!silent) 
                                output_stream << "文件" << (len1 < len2 ? "2" : "1") << "有多余字符" << endl;
                        }
                    }
                    else {
                        // 经 trim 后内容不同：按 EOF/长度差异报告（保持原语义）
                        if (in1.eof() != in2.eof()) {
                            if (!silent) {
                                if (in1.eof())
                                    output_stream << "文件1已结束/文件2仍有内容" << endl;
                                else
                                    output_stream << "文件2已结束/文件1仍有内容" << endl;
                            }
                        }
                        else if (len1 != len2) {
                            if (!silent) 
                                output_stream << "文件" << (len1 < len2 ? "2" : "1") << "有多余字符" << endl;
                        }
                        else {
                            if (!silent) 
                                output_stream << "行结束符不同" << endl;
                        }
                    }
                }
                else {
                    if (in1.eof() != in2.eof()) {
                        if (!silent) {
                            if (in1.eof())
                                output_stream << "文件1已结束/文件2仍有内容" << endl;
                            else
                                output_stream << "文件2已结束/文件1仍有内容" << endl;
                        }
                    }
                    else if (len1 != len2) {
                        if (!silent) output_stream << "文件" << (len1 < len2 ? "2" : "1") << "有多余字符" << endl;
                    }
                }
            }
            // 先将非颜色部分写入 buffer
            if (!silent)
                result();

            string disp_s1, disp_s2;
            if (display_type == "detailed" || display_type == "normal") {

                string disp_raw1 = raw1;
                if (!disp_raw1.empty() && disp_raw1.back() == '\r') 
                    disp_raw1.pop_back();
                string disp_raw2 = raw2;
                if (!disp_raw2.empty() && disp_raw2.back() == '\r')
                    disp_raw2.pop_back();

                disp_s1 = disp_raw1;
                disp_s2 = disp_raw2;
                trim_accordingly(disp_s1, trim_type);
                trim_accordingly(disp_s2, trim_type);

                int display_len1 = static_cast<int>(disp_s1.length());
                int display_len2 = static_cast<int>(disp_s2.length());
                int maxlen = (display_len1 > display_len2) ? display_len1 : display_len2;
                int ruler_len = (maxlen / 10 + 2) * 10 + 1;

                if (!silent) {
                    cout << "        " << string(ruler_len, '-') << endl;
                    cout << "        ";
                    for (int k = 0; k < ruler_len / 10; ++k)
                        cout << k % 10 << "         ";
                    cout << ruler_len / 10 % 10;
                    cout << endl;
                    cout << "        ";
                    for (int k = 0; k < ruler_len / 10; ++k)
                        cout << "0123456789";
                    cout << '0';
                    cout << endl;
                    cout << "        " << string(ruler_len, '-') << endl;
                }
            }

            if (!silent) {
                cout << "文件1 : ";
                print_diff_line(s1.c_str(), raw1, in1.eof(), (len1 < len2 ? len1 : len2), s2.c_str());
                cout << endl;

                cout << "文件2 : ";
                print_diff_line(s2.c_str(), raw2, in2.eof(), (len1 < len2 ? len1 : len2), s1.c_str());
                cout << endl;
            }

            if (display_type == "detailed") {
                if (!silent) {
                    cout << "文件1(HEX) : " << endl;
                }
                if (bin_open) {
                    if (trim_type == "none") {
                        if (!silent)
                            change_line_to_hex(raw1.c_str(), in1.eof());
                    }
                    else {
                        string disp_raw1 = raw1;
                        if (!disp_raw1.empty() && disp_raw1.back() == '\r') 
                            disp_raw1.pop_back();
                        string disp_s1_local = disp_raw1;
                        trim_accordingly(disp_s1_local, trim_type);

                        string hex_src = disp_s1_local;
                        bool hadCR = (!raw1.empty() && raw1.back() == '\r');
                        if (hadCR) 
                            hex_src.push_back('\r');
                        if (!silent) 
                            change_line_to_hex(hex_src.c_str(), in1.eof());
                    }
                }
                else {
                    if (trim_type == "none") {
                        if (!silent) 
                            change_line_to_hex(s1.c_str(), in1.eof());
                    }
                    else {
                        string disp_raw1 = raw1;
                        if (!disp_raw1.empty() && disp_raw1.back() == '\r') 
                            disp_raw1.pop_back();
                        string disp_s1_local = disp_raw1;
                        trim_accordingly(disp_s1_local, trim_type);
                        if (!silent)
                            change_line_to_hex(disp_s1_local.c_str(), in1.eof());
                    }
                }

                if (!silent) {
                    cout << "文件2(HEX) : " << endl;
                }
                if (bin_open) {
                    if (trim_type == "none") {
                        if (!silent) 
                            change_line_to_hex(raw2.c_str(), in2.eof());
                    }
                    else {
                        string disp_raw2 = raw2;
                        if (!disp_raw2.empty() && disp_raw2.back() == '\r')
                            disp_raw2.pop_back();
                        string disp_s2_local = disp_raw2;
                        trim_accordingly(disp_s2_local, trim_type);

                        string hex_src = disp_s2_local;
                        bool hadCR = (!raw2.empty() && raw2.back() == '\r');
                        if (hadCR) 
                            hex_src.push_back('\r');
                        if (!silent) 
                            change_line_to_hex(hex_src.c_str(), in2.eof());
                    }
                }
                else {
                    if (trim_type == "none") {
                        if (!silent) 
                            change_line_to_hex(s2.c_str(), in2.eof());
                    }
                    else {
                        string disp_raw2 = raw2;
                        if (!disp_raw2.empty() && disp_raw2.back() == '\r') 
                            disp_raw2.pop_back();
                        string disp_s2_local = disp_raw2;
                        trim_accordingly(disp_s2_local, trim_type);
                        if (!silent) 
                            change_line_to_hex(disp_s2_local.c_str(), in2.eof());
                    }
                }
            }

            if (!silent) 
                cout << endl;
        }
    }

    if (diff_count == 0 || is_offset_done) {
        bool remainedDiff = false;
        if (in1.eof() != in2.eof()) {
            if (ignore_blank) {
                if (!in1.eof()) {
                    streampos pos = in1.tellg();
                    streampos binPos = (bin_open ? bin1->tellg() : streampos(-1));
                    string tmp; 
                    bool found = false;
                    while (getline(in1, tmp)) {
                        trim_accordingly(tmp, trim_type);
                        if (tmp != "") { 
                            found = true;
                            break; 
                        }
                    }
                    in1.clear();
                    in1.seekg(pos);
                    if (bin_open) { 
                        bin1->clear(); 
                        bin1->seekg(binPos);
                    }
                    if (found) 
                        remainedDiff = true;
                }
                else if (!in2.eof()) {
                    streampos pos = in2.tellg();
                    streampos binPos = (bin_open ? bin2->tellg() : streampos(-1));
                    string tmp;
                    bool found = false;
                    while (getline(in2, tmp)) {
                        trim_accordingly(tmp, trim_type);
                        if (tmp != "") { 
                            found = true; 
                            break; 
                        }
                    }
                    in2.clear(); 
                    in2.seekg(pos);
                    if (bin_open) { 
                        bin2->clear(); 
                        bin2->seekg(binPos);
                    }
                    if (found)
                        remainedDiff = true;
                }
            }
            else
                remainedDiff = true;
        }

        if (remainedDiff) {
            if (display_type == "none") {
                if (!silent) 
                    output_stream << "文件不同." << endl;
            }
            else {
                {
                    if (!silent) {
                        output_stream << "第[" << line1_num << " / " << line2_num << "]行 - ";
                        output_stream << "行结束符不同" << endl;
                        result();
                    }

                    string next1, next2;
                    bool peek1_ok = false, peek2_ok = false;

                    // 优先用二进制流 peek（保留真实 '\r' 信息），读完后恢复 bin 位置
                    if (!in1.eof() && bin_open) {
                        streampos binPos1 = bin1->tellg();
                        string tmpraw;
                        bool ok = static_cast<bool>(getline(*bin1, tmpraw));
                        if (ok) {
                            raw1 = tmpraw;                  // 原始字节行（可能含 '\r'）
                            next1 = tmpraw;
                            if (!next1.empty() && next1.back() == '\r')
                                next1.pop_back(); // 文本形式
                            peek1_ok = true;
                        }
                        else {
                            raw1.clear();
                            next1.clear();
                        }
                        bin1->clear();
                        bin1->seekg(binPos1);
                    }

                    // 若二进制不可用，再尝试用文本流 peek（仅当 tellg 有效时）
                    if (!in1.eof() && !peek1_ok) {
                        streampos pos1 = in1.tellg();
                        if (pos1 != streampos(-1)) {
                            string tmp;
                            if (getline(in1, tmp)) {
                                next1 = tmp;
                                if (!next1.empty() && next1.back() == '\r') 
                                    next1.pop_back();
                                // raw1 保持为空（表示没有二进制原始数据）
                                peek1_ok = true;
                            }
                            in1.clear();
                            in1.seekg(pos1);
                        }
                    }

                    // file2 同理
                    if (!in2.eof() && bin_open) {
                        streampos binPos2 = bin2->tellg();
                        string tmpraw;
                        bool ok = static_cast<bool>(getline(*bin2, tmpraw));
                        if (ok) {
                            raw2 = tmpraw;
                            next2 = tmpraw;
                            if (!next2.empty() && next2.back() == '\r') 
                                next2.pop_back();
                            peek2_ok = true;
                        }
                        else {
                            raw2.clear();
                            next2.clear();
                        }
                        bin2->clear();
                        bin2->seekg(binPos2);
                    }

                    if (!in2.eof() && !peek2_ok) {
                        streampos pos2 = in2.tellg();
                        if (pos2 != streampos(-1)) {
                            string tmp;
                            if (getline(in2, tmp)) {
                                next2 = tmp;
                                if (!next2.empty() && next2.back() == '\r') 
                                    next2.pop_back();
                                peek2_ok = true;
                            }
                            in2.clear();
                            in2.seekg(pos2);
                        }
                    }

                    // 现在准备显示内容，bufX 用于可视化文本（已经去掉 CR），show_str 优先使用 raw（若有）以保证行尾判断一致
                    string buf1 = next1;
                    string buf2 = next2;
                    trim_accordingly(buf1, trim_type);
                    trim_accordingly(buf2, trim_type);
                    int len1 = (int)buf1.length();
                    int len2 = (int)buf2.length();
                    int minlen = (len1 < len2) ? len1 : len2;

                    if (display_type == "detailed" || display_type == "normal") {
                        int ruler_len = ((int)max(strlen(next1.c_str()), strlen(next2.c_str())) / 10 + 2) * 10 + 1;
                        if (!silent) {
                            cout << "        " << string(ruler_len, '-') << endl;
                            cout << "        ";
                            for (int k = 0; k < ruler_len / 10; ++k)
                                cout << k % 10 << "         ";
                            cout << ruler_len / 10 % 10;
                            cout << endl;
                            cout << "        ";
                            for (int k = 0; k < ruler_len / 10; ++k)
                                cout << "0123456789";
                            cout << '0';
                            cout << endl;
                            cout << "        " << string(ruler_len, '-') << endl;
                        }
                    }

                    if (!silent) {
                        cout << "文件1 : ";
                        if (in1.eof()) 
                            cout << "<EOF>";
                        else {
                            const string& show_str1 = (!raw1.empty() ? raw1 : next1); // 优先原始 raw（含 '\r'）
                            print_diff_line(buf1.c_str(), show_str1, in1.eof(), minlen, buf2.c_str());
                        }
                        cout << endl;

                        cout << "文件2 : ";
                        if (in2.eof()) 
                            cout << "<EOF>";
                        else {
                            const string& show_str2 = (!raw2.empty() ? raw2 : next2);
                            print_diff_line(buf2.c_str(), show_str2, in2.eof(), minlen, buf1.c_str());
                        }
                        cout << endl;

                        if (display_type == "detailed") {
                            cout << "文件1(HEX) : " << endl;
                            if (!raw1.empty())
                                change_line_to_hex(raw1.c_str(), in1.eof());
                            else
                                change_line_to_hex(next1.c_str(), in1.eof());

                            cout << "文件2(HEX) : " << endl;
                            if (!raw2.empty())
                                change_line_to_hex(raw2.c_str(), in2.eof());
                            else
                                change_line_to_hex(next2.c_str(), in2.eof());
                            cout << endl;
                        }
                    }
                }
            }
            diff_count++;
        }
    }

    if (diff_count == 0) {
        if (display_type == "none") {
            if (!silent) 
                output_stream << "文件相同." << endl;
        }
        else {
            if (!silent) {
                output_stream << "在指定检查条件下完全一致." << endl;
                output_stream << string(width, '=') << endl;
            }
        }
    }
    else if (display_type != "none") {
        if (!silent) {
            output_stream << string(width, '=') << endl;
            output_stream << "在指定检查条件下共" << diff_count << "行有差异";
            if (max_diffnum > 0 && diff_count >= max_diffnum)
                output_stream << "[已到设定的最大差异值]";
            output_stream << "." << endl;
            output_stream << "阅读提示：" << endl;
            output_stream << "\t1、每行的行结束符用<CR>/<LF>/<CR><LF>/<EOF>标出(方便看清行结束符的类型)" << endl;
            output_stream << "\t2、如果每行仅有<CR>/<LF>/<CR><LF>/<EOF>，则表示空行" << endl;
            output_stream << "\t3、文件结束标记为<EOF>" << endl;
            output_stream << "\t4、两行相同列位置的差异字符用亮色标出" << endl;
            output_stream << "\t5、每行中的CR/LF/VT/BS/BEL用X标出(方便看清隐含字符)" << endl;
            output_stream << "\t6、每行尾的多余的字符用亮色标出，VT/BS/BEL用亮色X标出(方便看清隐含字符)" << endl;
            output_stream << "\t7、中文因为编码问题，差异位置可能报在后半个汉字上，但整个汉字都亮色标出" << endl;
            if (display_type == "normal")
                output_stream << "\t8、用--display detailed可以得到更详细的信息" << endl;
            output_stream << string(width, '=') << endl;
        }
    }

    if (!silent)
        result();

    return diff_count;
}

void txt_compare::print_diff_line(const char* s, const string& str, bool eof, int minlen, const char* s_other)
{
    // 根据新版规则：不再用 'X' 表示行内的 CR/LF（由行尾标记单独显示）。
    // 仍保留对其它不可见控制字符（如 '\t','\b','\a','\v'）用 'X' 的处理以便可见化。
    if (!(str.empty() && !eof)) {
        int len = static_cast<int>(strlen(s));
        int other_len = static_cast<int>(strlen(s_other));
        for (int i = 0; i < len; ++i) {
            bool is_diff = (i >= minlen) || (i < other_len && s[i] != s_other[i]);
            if (is_diff)
                cct_setcolor(COLOR_HYELLOW, COLOR_HRED);

            unsigned char ch = static_cast<unsigned char>(s[i]);
            if (ch == '\r' || ch == '\n') {
                // 新版2.0.3要求：不要用 'X' 表示 CR/LF，直接跳过，让行尾的标记（<CR><LF>/<LF>）显示类型
            }
            else if (ch == '\t' || ch == '\b' || ch == '\a' || ch == '\v') {
                // 仍然用 'X' 标示这些隐含字符（便于查看）
                cout << 'X';
            }
            else {
                cout << s[i];
            }

            if (is_diff)
                cct_setcolor();
        }
    }

    // 行结束符显示：EOF / CRLF / LF（与现有逻辑一致）
    if (eof)
        cout << "<EOF>";
    else {
        if (!str.empty() && str.back() == '\r')
            cout << "<CR><LF>";
        else
            cout << "<LF>";
    }
}

void txt_compare::result()
{
    cout << output_stream.str();
    output_stream.str("");//第一次调用后清空缓冲
    output_stream.clear();
}