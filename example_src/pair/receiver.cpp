#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

int main() {
    int sock = nn_socket(AF_SP, NN_PAIR);
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

    std::cout << "nanoserver started, waiting for connection..." << std::endl;

    while (true) {
        char buf[1024] = {0};
        int bytes = nn_recv(sock, buf, sizeof(buf), 0);
        if (bytes > 0) {
            std::cout << "Received message: " << buf << std::endl;
        }
    }

    nn_close(sock);
    return 0;
}