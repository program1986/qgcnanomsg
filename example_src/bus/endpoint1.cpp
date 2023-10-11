#include <thread>
#include <chrono>

#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>

int main() {
    int sock = nn_socket(AF_SP, NN_BUS);
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

    int connect_result = nn_connect(sock, "tcp://127.0.0.1:5556");
    if (connect_result < 0) {
        std::cerr << "Error connecting socket: " << nn_strerror(nn_errno()) << std::endl;
        nn_close(sock);
        return -1;
    }

    char buf[1024] = {0};
    while (true) {
        std::cout << "Enter a message to send to Endpoint 2: ";
        std::cin.getline(buf, sizeof(buf));

        int bytes = nn_send(sock, buf, strlen(buf), NN_DONTWAIT);
        if (bytes > 0) {
            std::cout << "Endpoint 1 sent message: " << buf << std::endl;
        }

        int recv_bytes = nn_recv(sock, buf, sizeof(buf), NN_DONTWAIT);
        if (recv_bytes > 0) {
            std::cout << "Endpoint 1 received message: " << buf << std::endl;
        }
        
    }

    nn_close(sock);
    return 0;
}