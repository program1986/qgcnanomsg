#include <thread>
#include <chrono>
#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

int main() {
    int sock = nn_socket(AF_SP, NN_SUB);
    if (sock < 0) {
        std::cerr << "Error creating socket: " << nn_strerror(nn_errno()) << std::endl;
        return -1;
    }

    int connect_result = nn_connect(sock, "tcp://127.0.0.1:5555");
    if (connect_result < 0) {
        std::cerr << "Error connecting socket: " << nn_strerror(nn_errno()) << std::endl;
        nn_close(sock);
        return -1;
    }

    // 订阅所有消息
    nn_setsockopt(sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);

    char buf[1024] = {0};
    while (true) {
        int bytes = nn_recv(sock, buf, sizeof(buf), 0);
        if (bytes > 0) {
            std::cout << "Received message: " << buf << std::endl;
        }
    }

    nn_close(sock);
    return 0;
}