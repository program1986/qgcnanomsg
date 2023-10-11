/**
 * @file nanomsg_server.cpp
 * @brief  QGC 服务模拟程序，用于调试。
 * @details
 * 本程序共建立两个服务。一个pubsub用于发布状态，一个req，用于接收命令。
 * @author shijingwei
 * @version 0.1
 * @date 2023-10-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <thread>

#include <json/json.h>
#include <nanomsg/bus.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>


using namespace std;

#define BUF_LEN 100
#define SUCCESS 0  // 操作成功

/**------------------数据定义-----------------------------------**/

// 状态数据
std::string statusString = R"(
    {
        "header": 1235,
        "timestamp": 100000000,
        "serial": 5,
        "cmd": 1,
        "data": {
            "alarm_info": "Target lost",
            "object_num": 2,
            "object_array": [
                {
                    "object_id": 12,
                    "object_name": "person",
                    "object_conf": 0.99,
                    "rect_color": "#ff0000",
                    "rect_x": 0,
                    "rect_y": 0,
                    "rect_width": 100,
                    "rect_height": 100
                },
                {
                    "object_id": 4,
                    "object_name": "car",
                    "object_conf": 0.83,
                    "rect_color": "#0000ff",
                    "rect_x": 0,
                    "rect_y": 0,
                    "rect_width": 100,
                    "rect_height": 100
                }
            ]
        }
    }
)";

//回复指令
std::string jsonReplay = R"(
{
    "header": 1234,
    "serial": 0,
    "cmd": 1,
    "code": 512,
    "data": {
        "result_1": 0,
        "result_2": "yes"
    }
}
)";

/*--------------------函数定义-------------------------------*/

/**
 * @brief 建立发布服务
 *
 * @param url 发布服务地址
 */
void createPubService(const char *url) {
  int sock = nn_socket(AF_SP, NN_PUB);
  if (sock < 0) {
    std::cerr << "Error creating socket: " << nn_strerror(nn_errno())
              << std::endl;
    return;
  }

  int bind_result = nn_bind(sock, url);
  if (bind_result < 0) {
    std::cerr << "Error binding socket: " << nn_strerror(nn_errno())
              << std::endl;
    nn_close(sock);
    return;
  }

  while (true) {
    int bytes = nn_send(sock, statusString.c_str(), statusString.size(), 0);
    if (bytes > 0) {
      std::cout << "Sending success." << std::endl;
    }

    // 按照一秒10次发送
    std::this_thread::sleep_for(
        std::chrono::nanoseconds(100 * 1000));  // 每秒发送一次请求
  }

  nn_close(sock);
}

/**
 * @brief 建立命令接收服务
 * 
 * @param url 服务地址
 */
void createRespondService(const char *url) {

    int sock = nn_socket(AF_SP, NN_REP);
    if (sock < 0) {
        std::cerr << "Error creating socket: " << nn_strerror(nn_errno()) << std::endl;
        return;
    }

    int bind_result = nn_bind(sock, url);
    if (bind_result < 0) {
        std::cerr << "Error binding socket: " << nn_strerror(nn_errno()) << std::endl;
        nn_close(sock);
        return;
    }

    while (true) {
        char buf[BUF_LEN] = {0};
        int bytes = nn_recv(sock, buf, sizeof(buf), 0);
        if (bytes > 0) {
            std::cout << "Received request: " << buf << std::endl;

            std::string reply = jsonReplay;
            int send_result = nn_send(sock, reply.c_str(), reply.size(), 0);
            if (send_result < 0) {
                std::cerr << "Error sending reply: " << nn_strerror(nn_errno()) << std::endl;
            }
        }
    }

    nn_close(sock);
}

// 接收线程
int cmd_recv_func(int sock) {
  int rv;
  char buf[BUF_LEN];
  int bytes;

  while (1) {
    memset(buf, 0, BUF_LEN);
    bytes = nn_recv(sock, &buf, BUF_LEN, 0);
    if (bytes) {
      printf("recieve client msg: %s\r\n", buf);
    }
  }
  return (nn_shutdown(sock, rv));
}


int main(int argc, char **argv) {
  int server_sock = 0;
  char buf[BUF_LEN] = {0};
  int option;
  int port;
  string hostAddress;
  string connectionString;

 
  // 检查命令行参数
  if (argc < 2) {
    hostAddress = "127.0.0.1";
    port = 2021;
    connectionString = "tcp://" + hostAddress + ":" + std::to_string(port);
    cout << "Using the default connection:" << connectionString << endl;
  }

  while ((option = getopt(argc, argv, "h:p:")) != -1) {
    switch (option) {
      case 'h':
        hostAddress = optarg;
        break;
      case 'p':
        try {
          port = std::stoi(optarg);
        } catch (...) {
          std::cout << "Invalid value for -p option." << std::endl;
          return 1;
        }
        break;
      case '?':
        std::cout << "Unknown option: " << optopt << std::endl;
        return 1;
    }
  }

  //url 获取
  connectionString = "tcp://" + hostAddress + ":" + std::to_string(port);
  cout << "Connection is " << connectionString << endl;

  if ((server_sock = nn_socket(AF_SP, NN_PAIR)) < 0) {
    cout << "Create server socket failed!" << endl;
    return -1;
  }

  if (nn_bind(server_sock, connectionString.c_str()) < 0) {
    cout << "Bind server sock failed!" << endl;
    nn_close(server_sock);
    return -1;
  }

  cout << "Server init success!" << endl;

  // 启动接收线程
  // std::thread cmd_thread{cmd_recv_func, server_sock};
  std::thread pub_thread{createPubService, "tcp://127.0.0.1:5555"};

  while (1) {
    // printf("%s\n", json);

    std::this_thread::sleep_for(
        std::chrono::nanoseconds(1000 * 1000));  // 每秒发送一次请求

    // std::string jsonInput;
    // std::cin >> jsonInput;

    // std::cout << jsonInput << std::endl;
    //  if (jsonInput == "exit"){
    //    break;
    //  }

    /*
    if (jsonInput.empty() == false) {
      if (nn_send(server_sock, jsonInput.c_str(), jsonInput.length(), 0) < 0) {
        printf("send failed!\r\n");
        nn_close(server_sock);
        exit(EXIT_FAILURE);
      }
    }
    */
  }

  nn_close(server_sock);
  // cmd_thread.native_handle();
  // cout << "Service terminated normally." << endl;
  return 0;
}

/*-------------------测试函数----------------------------*/

/**
 * @brief 测试json解析函数
 * 
 */
void jsonExtraExample() {
  const std::string json = R"(
        {
            "name": "John",
            "age": 30,
            "city": "New York"
        }
    )";

  // 创建Json::Value对象
  Json::Value root;

  // 解析JSON字符串
  Json::CharReaderBuilder reader;
  std::istringstream iss(json);
  std::string errs;
  Json::parseFromStream(reader, iss, &root, &errs);

  // 获取JSON字段的值
  std::string name = root["name"].asString();
  int age = root["age"].asInt();
  std::string city = root["city"].asString();

  // 打印结果
  std::cout << "Name: " << name << std::endl;
  std::cout << "Age: " << age << std::endl;
  std::cout << "City: " << city << std::endl;
}

/**
 * @brief Json 生成测试
 * 
 */
void jsonGenerateExample() {
  Json::Value root;

  Json::Value data;
  data["mode"] = 0;
  data["status"] = 0;

  root["header"] = 1234;
  root["serial"] = 5;
  root["event"] = 2;
  root["data"] = data;

  // 输出json串
  Json::StreamWriterBuilder writer;

  std::string json = Json::writeString(writer, root);

  std::cout << json << std::endl;
}
