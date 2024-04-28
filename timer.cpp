#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

class Timer {
private:
    const double defaultT = -1; // Ĭ�ϵ�ʱ���
    double stTime;  // ��ʼʱ��
    double edTime;  // ����ʱ��

public:
    // ��ʼ����ʱ��
    Timer(double duration) : stTime(defaultT), edTime(duration) {}

    // ��ʼ��ʱ
    void start() {
        if (stTime == defaultT) {
            stTime = time(nullptr);
        }
    }

    // ֹͣ��ʱ
    void stop() {
        if (stTime != defaultT) {
            stTime = defaultT;
        }
    }

    // �жϼ�ʱ���Ƿ��ڼ�ʱ
    bool running() {
        return stTime != defaultT;
    }

    // �ж��Ƿ�ʱ
    bool timeout() {
        if (!running()) {
            return false;
        } else {
            return time(nullptr) - stTime >= edTime;
        }
    }
};

int main() {
    using namespace std;

    // ����һ����ʱ�����󲢲���
    Timer timer(5); // ���ó���ʱ��Ϊ5��

    timer.start(); // ��ʼ��ʱ

    while (true) {

        // ����Ƿ�ʱ
        if (timer.timeout()) {
            cout << "Timeout!" << endl;
            break;
        }

        // �ȴ�һ��ʱ��
        cout << "Waiting..." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
