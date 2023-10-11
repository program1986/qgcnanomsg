#include <iostream>
#include <thread>
#include <chrono>

#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>

int main() {
    int sock = nn_socket(AF_SP, NN_REQ);
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
        std::string request = "Request " + std::to_string(sequence);
        int send_result = nn_send(sock, request.c_str(), request.size(), 0);
        if (send_result < 0) {
            std::cerr << "Error sending request: " << nn_strerror(nn_errno()) << std::endl;
            break;
        }

        char buf[1024] = {0};
        int bytes = nn_recv(sock, buf, sizeof(buf), 0);
        if (bytes > 0) {
            std::cout << "Received reply: " << buf << std::endl;
        }

        ++sequence;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒发送一次请求
    }

    nn_close(sock);
    return 0;
}