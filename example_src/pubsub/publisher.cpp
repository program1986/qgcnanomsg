#include <thread>
#include <chrono>
#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

int main() {
    int sock = nn_socket(AF_SP, NN_PUB);
    if (sock < 0) {
        std::cerr << "Error creating socket: " << nn_strerror(nn_errno()) << std::endl;
        return -1;
    }

    int bind_result = nn_bind(sock, "tcp://127.0.0.1:5555");
    if (bind_result < 0) {
        std::cerr << "Error binding socket: " << nn_strerror(nn_errno()) << std::endl;
        nn_close(sock);
        return -1;
    }

    int sequence = 1;
    while (true) {
        // 将序列数字转换为字符串
        std::string message = std::to_string(sequence);

        int bytes = nn_send(sock, message.c_str(), message.size(), 0);
        if (bytes > 0) {
            std::cout << "Published message: " << message << std::endl;
        }

        ++sequence;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒发送一次请求
    
    }

    nn_close(sock);
    return 0;
}