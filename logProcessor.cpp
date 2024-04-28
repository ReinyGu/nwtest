#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: ./logProcessor [logfile.txt]" << endl;
        return 0;
    }

    // 打开日志文件
    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cout << "Error opening file." << endl;
        return 0;
    }

    // 读取文件中的每一行
    string line;
    int cnt_tot = 0, cnt_err = 0;
    while (getline(infile, line)) {
        // 拆分每一行的日志条目
        string log1, log2, log3, log4;
        size_t pos = 0;
        pos = line.find(',');
        log1 = line.substr(0, pos);
        line.erase(0, pos + 1);
        pos = line.find(',');
        log2 = line.substr(0, pos);
        line.erase(0, pos + 1);
        pos = line.find(',');
        log3 = line.substr(0, pos);
        line.erase(0, pos + 1);
        log4 = line;

        // 解析日志条目的内容
        string MsgType, UDP_recv, status, ackedNo;
        size_t eq_pos = log2.find('=');
        MsgType = log2.substr(0, eq_pos);
        UDP_recv = log2.substr(eq_pos + 1);
        eq_pos = log3.find('=');
        status = log3.substr(eq_pos + 1);
        eq_pos = log4.find('=');
        ackedNo = log4.substr(eq_pos + 1);

        // 检查日志类型并计数错误日志
        if (MsgType == "UDP_exp") {
            continue; // receiver的日志
        }
        cnt_tot++;
        if (status == "RT" || status == "TO") {
            cnt_err++;
        }
    }

    // 计算并输出错误率
    if (cnt_tot > 0) {
        double error_rate = static_cast<double>(cnt_err) / cnt_tot * 100;
        cout << "错误率：" << error_rate << "%" << endl;
    } else {
        cout << "日志文件为空或格式不正确。" << endl;
    }

    // 关闭文件
    infile.close();

    return 0;
}
