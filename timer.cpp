#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

class Timer {
private:
    const double defaultT = -1; // 默认的时间戳
    double stTime;  // 开始时间
    double edTime;  // 持续时间

public:
    // 初始化计时器
    Timer(double duration) : stTime(defaultT), edTime(duration) {}

    // 开始计时
    void start() {
        if (stTime == defaultT) {
            stTime = time(nullptr);
        }
    }

    // 停止计时
    void stop() {
        if (stTime != defaultT) {
            stTime = defaultT;
        }
    }

    // 判断计时器是否在计时
    bool running() {
        return stTime != defaultT;
    }

    // 判断是否超时
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

    // 创建一个计时器对象并测试
    Timer timer(5); // 设置持续时间为5秒

    timer.start(); // 开始计时

    while (true) {

        // 检查是否超时
        if (timer.timeout()) {
            cout << "Timeout!" << endl;
            break;
        }

        // 等待一段时间
        cout << "Waiting..." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
