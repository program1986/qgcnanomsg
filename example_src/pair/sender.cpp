#include <iostream>
#include <thread>
#include <chrono>

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

int main() {
    int sock = nn_socket(AF_SP, NN_PAIR);
    if (sock < 0) {
        std::cerr << "Error creating socket: " << nn_strerror(nn_errno()) << std::endl;
        return -1;
    }

    int connect_result = nn_connect(sock, "tcp://127.0.0.1:5555");
    if (connect_result < 0) {
        std::cerr << "Error connecting to socket: " << nn_strerror(nn_errno()) << std::endl;
        nn_close(sock);
        return -1;
    }

    int sequence = 0;
    while (true) {
        std::string message = "Sequence: " + std::to_string(sequence);
        int send_result = nn_send(sock, message.c_str(), message.size(), 0);
        if (send_result < 0) {
            std::cerr << "Error sending message: " << nn_strerror(nn_errno()) << std::endl;
            break;
        } else {
            std::cout << "Message sent: " << message << std::endl;
        }

        ++sequence;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒发送一次
    }

    nn_close(sock);
    return 0;
}