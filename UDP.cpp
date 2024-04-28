#include <iostream>
#include <vector>
#include <tuple> 
#include <cstdint>
using namespace std;

class UDP {
public:
    // UDP() = delete; // 禁用默认构造函数

    static vector<uint8_t> make(int frame_nr, int frame_expected, int data_value, int MAX_SEQ, const vector<uint8_t>& data);
    static tuple<int, int, int, vector<uint8_t>, int> extract(const vector<uint8_t>& packet);
    static int CRCCCITT(const vector<uint8_t>& bitstr);
};




// CRC-CCITT 校验函数
int UDP::CRCCCITT(const vector<uint8_t>& bitstr) {
    // 定义CRC-CCITT标准的位置
    vector<int> loc = {16, 12, 5, 0};
    vector<int> p(17, 0);

    // 初始化生成多项式
    for (int i : loc) {
        p[16 - i] = 1; // 反转，p[0]表示16次方，以此类推
    }

    vector<int> info_list;
    // 将输入的字节流转换成比特串
    for (uint8_t xstr : bitstr) {
        vector<int> bin_s(8, 0);
        int bin_s_len = 0;
        while (xstr > 0) {
            bin_s[bin_s_len] = xstr % 2;
            xstr /= 2;
            bin_s_len++;
        }
        for (int i = 0; i < 8 - bin_s_len; i++) {
            info_list.push_back(0);
        }
        for (int j : bin_s) {
            info_list.push_back(j);
        }
    }

    int strlen = info_list.size();

    // 在比特串后添加16个0
    for (int i = 0; i < 16; i++) {
        info_list.push_back(0);
    }

    // CRC-CCITT算法主循环
    for (int i = 0; i < strlen; i++) {
        if (info_list[i] == 1) {
            for (int j = 0; j < 17; j++) {
                info_list[j + i] = info_list[j + i] ^ p[j];
            }
        }
    }

    // 取最后16位作为校验码
    vector<int> check_code(info_list.end() - 16, info_list.end());
    int check_sum = 0;
    for (int i : check_code) {
        check_sum = (check_sum << 8) ^ i;
        for (int j = 0; j < 8; j++) {
            if (check_sum & 0x8000) {
                check_sum = (check_sum << 1) ^ 0x1021;
            } else {
                check_sum <<= 1;
            }
        }
    }
    return check_sum;
}

// 生成UDP包函数
vector<uint8_t> UDP::make(int frame_nr, int frame_expected, int data_value, int MAX_SEQ, const vector<uint8_t>& data) {
    int seq = frame_nr % MAX_SEQ;
    int ack = frame_expected % MAX_SEQ;
    // 构建UDP的头部
    vector<uint8_t> header = {(uint8_t)seq, (uint8_t)ack, (uint8_t)data_value};
    // 计算CRC-CCITT校验码
    vector<uint8_t> checksum(2, 0);
    int checksum_value = CRCCCITT(header);
    checksum[0] = (uint8_t)((checksum_value >> 8) & 0xFF);
    checksum[1] = (uint8_t)(checksum_value & 0xFF);
    // 将数据和校验码拼接到UDP中
    vector<uint8_t> UDP = header;
    UDP.insert(UDP.end(), data.begin(), data.end());
    UDP.insert(UDP.end(), checksum.begin(), checksum.end());
    return UDP;
}

// 解析UDP包函数
tuple<int, int, int, vector<uint8_t>, int> UDP::extract(const vector<uint8_t>& packet) {
    // 解析头部
    int seq = packet[0];
    int ack = packet[1];
    int data_value = packet[2];
    // 解析数据
    vector<uint8_t> data(packet.begin() + 3, packet.end() - 2);
    // 解析CRC-CCITT校验码
    int checksum = (packet[packet.size() - 2] << 8) + packet[packet.size() - 1];
    return make_tuple(seq, ack, data_value, data, checksum);
}

int main() {
    // // 测试1
    // // 创建UDP并输出每个字节的十六进制表示
    // vector<uint8_t> frame = UDP::make(2, 5, 2, 9, {'S', 'u', 'n', 'o', 'o', ' ', 'c', 'u', 't', 't', 'i', 'e'});
    // for (uint8_t byte : frame) {
    //     cout << hex << static_cast<int>(byte) << ' ';
    // }
    // cout << endl;

    // 测试2
    cout << "test of bitstr'1120212529'" << endl;
    cout << UDP::CRCCCITT({'1', '1', '2', '0', '2', '1', '2', '5', '2', '9'}) << endl;

    // 创建UDP并输出每个字节的十六进制表示
    vector<uint8_t> frame = UDP::make(2,5,2,9,{'1', '1', '2', '0', '2', '1', '2', '5', '2', '9'});
    for (uint8_t byte : frame) {
        cout << hex << static_cast<int>(byte) << ' ';
    }
    cout << endl;

    return 0;
}
