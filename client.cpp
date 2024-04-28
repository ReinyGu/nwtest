#include "UDP.cpp"
#include "utils.cpp"
#include "timer.cpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

using namespace std;

class CLIENT {
private:
    int MY_PORT;
    int DEST_PORT;
    float timeout;
    int MAX_SEQ;
    int SWS;
    int error_rate;
    int lost_rate;
    pair<string, int> MY_ADDR;
    pair<string, int> YOUR_ADDR;
    Timer send_timer;
    int nowack;
    int is_sending;
    int next_frame_to_send;
    int needRT;
    int needTO;
    int max_frame_ID;
    int frame_expected;
    int recv_state;
    int sock;
    ofstream logger;
    int logCount;

public:
    CLIENT(int My_Port, int Dest_Port, float _timeout = 0.03, int _MAX_SEQ = 8, int _SWS = 5, int _error_rate = 0, int _lost_rate = 0)
        : MY_PORT(My_Port), DEST_PORT(Dest_Port), timeout(_timeout), MAX_SEQ(_MAX_SEQ), SWS(_SWS), error_rate(_error_rate), lost_rate(_lost_rate),
          MY_ADDR(make_pair("localhost", My_Port)), YOUR_ADDR(make_pair("localhost", Dest_Port)),
          send_timer(_timeout), nowack(0), is_sending(0), next_frame_to_send(0), needRT(0), needTO(0), max_frame_ID(-1),
          frame_expected(0), recv_state(0), logCount(1) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(MY_PORT);
        bind(sock, (struct sockaddr*)&addr, sizeof(addr));
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        string log_filepath = "log_for_" + to_string(My_Port) + ".txt";
        logger.open(log_filepath);
    }

    ~CLIENT() {
        logger.close();
    }

    void clrset() {
        nowack = is_sending = next_frame_to_send = needRT = needTO = 0;
        max_frame_ID = -1;
    }

    int set_window_size(int num_packets) {
        return min(SWS, num_packets - 1 - nowack);
    }

    int inc(int num) {
        return (num + 1) % MAX_SEQ;
    }

    string getSenderMsg() {
        if (needRT == 1) {
            return to_string(logCount) + ",UDP_to_send=" + to_string(next_frame_to_send) + ",status=RT,ackedNo=" + to_string(nowack) + "\n";
        } else if (needTO == 1) {
            return to_string(logCount) + ",UDP_to_send=" + to_string(next_frame_to_send) + ",status=TO,ackedNo=" + to_string(nowack) + "\n";
        } else {
            return to_string(logCount) + ",UDP_to_send=" + to_string(next_frame_to_send) + ",status=New,ackedNo=" + to_string(nowack) + "\n";
        }
    }

    void sender(string filename) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cout << "Cannot open file: " << filename << endl;
            exit(0);
        }

        is_sending = 1;
        vector<string> packets;
        string data;
        while (getline(file, data)) {
            packets.push_back(data);
        }
        int num_packets = packets.size();
        cout << "This file can be divided into " << num_packets << " packets." << endl;
        int window_size = set_window_size(num_packets);

        while (nowack < num_packets) {
            while (next_frame_to_send < nowack + window_size && next_frame_to_send < num_packets) {
                vector<uint8_t> UDP_data = UDP::make(next_frame_to_send % MAX_SEQ, nowack % MAX_SEQ, 1, MAX_SEQ, vector<uint8_t>(packets[next_frame_to_send].begin(), packets[next_frame_to_send].end()));
                // string s = UDP::make(next_frame_to_send % MAX_SEQ, nowack % MAX_SEQ, 1, MAX_SEQ, packets[next_frame_to_send]);
                utils my_utils;
                my_utils.send(UDP_data, sock,  YOUR_ADDR, error_rate, lost_rate);
                if (next_frame_to_send > max_frame_ID) {
                    max_frame_ID = next_frame_to_send;
                    needTO = needRT = 0;
                }
                string SendMsg = getSenderMsg();
                logger.write(SendMsg.c_str(), SendMsg.length());
                logCount++;
                next_frame_to_send++;
                window_size = set_window_size(num_packets);
            }

            if (!send_timer.running()) {
                send_timer.start();
            }

            int flag = 0;
            while (send_timer.running() && !send_timer.timeout()) {
                if (next_frame_to_send < nowack + window_size || needRT == 1) {
                    flag = 1;
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(10));
            }

            if (flag == 1) {
                send_timer.stop();
                if (needRT == 1) {
                    next_frame_to_send = nowack;
                }
                continue;
            }

            if (send_timer.timeout()) {
                send_timer.stop();
                needTO = 1;
                next_frame_to_send = nowack;
            }
        }
        utils u;
        u.send("",sock , YOUR_ADDR, error_rate, lost_rate);
        clrset();
    }

    void get_Ack() {
        int flag = 0;
        while (true) {
            while (is_sending == 1) {
                flag = 1;
                try {
                    string _pkt;
                    pair<string, int> addr;
                    utils::recv(sock, _pkt, addr, 3);
                    int seq, ack, data_value;
                    tie(seq, ack, data_value) = UDP::extract_header(_pkt);
                    if (data_value == 1) {
                        continue;
                    }
                    string pkt;
                    utils::recv(sock, pkt, addr, 2);
                    if (seq == 0 && data_value == 0) {
                        nowack++;
                    } else if (seq == 1 && ack == nowack && data_value == 0) {
                        needRT = 1;
                    }
                } catch (...) {
                    continue;
                }
            }
            if (flag == 1 && is_sending == 0) {
                break;
            }
        }
    }

    void receiver() {
        int file_count = 0;
        ofstream file;
        while (true) {
            try {
                string pkt;
                pair<string, int> addr;
                utils::recv(sock, pkt, addr, 1029);
                if (pkt.empty()) {
                    file.close();
                    recv_state = 0;
                    frame_expected = 0;
                    continue;
                }

                int seq, ack, data_value, check_sum;
                string data;
                tie(seq, ack, data_value, data, check_sum) = UDP::extract(pkt);
                if (data_value == 0) {
                    if (seq == 0 && data_value == 0) {
                        nowack++;
                    } else if (seq == 1 && ack == nowack && data_value == 0) {
                        needRT = 1;
                    }
                    continue;
                } else if (data_value == 1 && recv_state == 0) {
                    recv_state = 1;
                    file.open("copy_from_" + to_string(YOUR_ADDR.second) + "_" + to_string(file_count) + ".txt", ios::binary);
                    file_count++;
                }

                int rev_check_sum = UDP::CRCCCITT(pkt.substr(0, pkt.length() - 2));
                if (seq != frame_expected) {
                    string NoErrorMsg = to_string(logCount) + ",UDP_exp=" + to_string(frame_expected) + ",UDP_recv=" + to_string(seq) + ",status=NoErr\n";
                    logger.write(NoErrorMsg.c_str(), NoErrorMsg.length());
                    logCount++;
                } else if (rev_check_sum == check_sum) {
                    frame_expected = inc(frame_expected);
                    string pkt_ack = UDP::make(0, frame_expected, 0, MAX_SEQ);
                    utils::send(sock, pkt_ack, YOUR_ADDR, "ack");
                    file.write(data.c_str(), data.length());
                    string OKMsg = to_string(logCount) + ",UDP_exp=" + to_string(seq) + ",UDP_recv=" + to_string(seq) + ",status=OK\n";
                    logger.write(OKMsg.c_str(), OKMsg.length());
                    logCount++;
                } else {
                    string pkt_dek = UDP::make(1, frame_expected, 0, MAX_SEQ);
                    utils::send(sock, pkt_dek, YOUR_ADDR, "dek");
                    string DataErrorMsg = to_string(logCount) + ",UDP_exp=" + to_string(frame_expected) + ",UDP_recv=" + to_string(seq) + ",status=DataErr\n";
                    logger.write(DataErrorMsg.c_str(), DataErrorMsg.length());
                    logCount++;
                }
            } catch (...) {
                continue;
            }
        }
    }
};
