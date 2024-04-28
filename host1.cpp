#include <iostream>
#include <string>
#include <thread>
#include "client.cpp"

using namespace std;

CLIENT *client1 = nullptr;
int file_count = 0;

void stop_threads() {
    for (auto& thread : thread::enumerate()) {
        thread.kill = true;
    }

    for (auto& thread : thread::enumerate()) {
        if (thread != thread::current_thread()) {
            thread.join();
        }
    }
}

int main() {
   

    int host_port, host_port1;
  
    cout << "Enter the host port:";
    cin >> host_port;
    cout << "Enter the destination port:";
    cin >> host_port1;
    

    int host_error_rate, host_loss_rate, host_max_seq, host_SWS;
    float host_timeout;
    cout << "Enter the error rate:  ";
    cin >> host_error_rate;
    cout << "Enter the loss rate: ";
    cin >> host_loss_rate;
    cout << "Enter the maximum sequence number: ";
    cin >> host_max_seq;
    cout << "Enter the SWS: ";
    cin >> host_SWS;
    cout << "Enter the timeout ";
    cin >> host_timeout;

    client1 = new CLIENT(host_port, host_port1, host_timeout, host_max_seq, host_SWS, host_error_rate, host_loss_rate);
    
    thread recv_thread(&CLIENT::receiver, client1);
    recv_thread.detach();

    while (true) {
        string filename;
        cout << "Enter the file path(Auto receiving, Or enter \"q\" to quit): ";
        cin >> filename;
        if (filename == "q") {
            stop_threads();
            break;
        }
        client1->sender(filename);
    }

    delete client1;
    return 0;
}
