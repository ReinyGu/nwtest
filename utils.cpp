#include <iostream>
#include <vector>
#include <random>
#include <tuple>
#include <cstring>
#include <sys/types.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <ws2tcpip.h>

using namespace std;

class utils {
public:
    // utils() = delete; // ����Ĭ�Ϲ��캯��

    void send(const vector<uint8_t>& packets, int sock, struct sockaddr_in& addr, double ErrorRate = 0, double LostRate = 0, string type = "data");
    tuple<vector<uint8_t>, struct sockaddr_in> recv(int sock, int max_size = 1029);
};




// �������ݰ�����
void utils::send(const vector<uint8_t>& packets, int sock, struct sockaddr_in& addr, double ErrorRate = 0, double LostRate = 0, string type = "data") {
    if (type != "data") {
        sendto(sock, reinterpret_cast<const char*>(packets.data()), packets.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
        return;
    }

    if (ErrorRate != 0 && rand() % static_cast<int>(100 / ErrorRate) == 0) {
        vector<uint8_t> errorArr = {0};
        vector<uint8_t> modifiedPacket = packets;
        modifiedPacket[packets.size() - 1] = 0; // �����һ���ֽ���Ϊ0
        sendto(sock, reinterpret_cast<const char*>(modifiedPacket.data()), modifiedPacket.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
        return;
    }

    if (LostRate != 0 && rand() % static_cast<int>(100 / LostRate) == 0) {
        return;
    }

    sendto(sock, reinterpret_cast<const char*>(packets.data()), packets.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
}

// �Ӳ��ɿ��ŵ��������ݰ�����
tuple<vector<uint8_t>, struct sockaddr_in> utils::recv(int sock, int max_size = 1029) {
    vector<uint8_t> packets(max_size);
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    ssize_t bytes_received = recvfrom(sock, reinterpret_cast<char*>(packets.data()), max_size, 0, (struct sockaddr*)&addr, &addr_len);
    packets.resize(bytes_received);
    return make_tuple(packets, addr);
}
