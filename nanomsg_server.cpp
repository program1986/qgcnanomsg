/**
 * @file nanomsg_server.cpp
 * @brief  QGC 服务模拟程序，用于调试。
 * @details
 * 本程序共建立两个服务。一个pubsub用于发布状态，一个req，用于接收命令。使用方法
 * nanoserver -h 服务器地址 -p 发布端口  -c 命令端口
 * 如：nanoserver -h 192.168.2.1 -p 5556 -c 5555
 * @author shijingwei
 * @version 0.1
 * @date 2023-10-05
 */

#include <json/json.h>
#include <nanomsg/bus.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <thread>

using namespace std;

#define BUF_LEN 100
#define SUCCESS 0  // 操作成功

/**------------------数据定义-----------------------------------**/

// 状态数据
std::string statusString1 = R"(
    {
        "header": 1235,
        "timestamp": 100000000,
        "serial": 5,
        "cmd": 1,
        "data": {
            "alarm_info": "",
            "object_num": 2,
            "object_array": [
                {
                    "object_id": 12,
                    "object_name": "person",
                    "object_conf": 0.99,
                    "rect_color": "#ff0000",
                    "rect_x": 400,
                    "rect_y": 100,
                    "rect_width": 200,
                    "rect_height": 200
                },
                {
                    "object_id": 4,
                    "object_name": "car",
                    "object_conf": 0.83,
                    "rect_color": "#0000ff",
                    "rect_x": 300,
                    "rect_y": 300,
                    "rect_width": 50,
                    "rect_height": 50
                }
            ]
        }
    }
)";

std::string statusString2 = R"(
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
                    "rect_x": 300,
                    "rect_y": 300,
                    "rect_width": 50,
                    "rect_height": 50
                },
                {
                    "object_id": 4,
                    "object_name": "dog",
                    "object_conf": 0.83,
                    "rect_color": "#00ffff",
                    "rect_x": 150,
                    "rect_y": 150,
                    "rect_width": 60,
                    "rect_height": 60
                }
            ]
        }
    }
)";

// 回复指令
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

  int count =0;
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
    int bytes;
    count ++;
    if (count %2 ==0)
      nn_send(sock, statusString1.c_str(), statusString1.size(), 0);
    else
      nn_send(sock, statusString2.c_str(), statusString2.size(), 0);
    if (bytes > 0) {
      // std::cout << "Sending success." << std::endl;
    }

    // 按照一秒10次发送
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // 每秒发送一次请求
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
    char buf[BUF_LEN] = {0};
    int bytes = nn_recv(sock, buf, sizeof(buf), 0);
    if (bytes > 0) {
      std::cout << "Received request: " << buf << std::endl;

      std::string reply = jsonReplay;
      int send_result = nn_send(sock, reply.c_str(), reply.size(), 0);
      if (send_result < 0) {
        std::cerr << "Error sending reply: " << nn_strerror(nn_errno())
                  << std::endl;
      }
    }
  }

  nn_close(sock);
}

int main(int argc, char **argv) {
  int server_sock = 0;
  int option;

  // 服务器地址
  string hostAddress = "";
  // 发布服务器端口
  int publishPort = -1;
  // 命令服务器端口
  int commandPort = -1;

  string publishConnectionString = "";
  string commandConnectionString = "";

  // 检查命令行参数
  if (argc < 2) {
    hostAddress = "127.0.0.1";
    publishPort = 5556;
    commandPort = 5555;

    commandConnectionString =
        "tcp://" + hostAddress + ":" + std::to_string(commandPort);
    publishConnectionString =
        "tcp://" + hostAddress + ":" + std::to_string(publishPort);
  } else {
    while ((option = getopt(argc, argv, "h:c:p:")) != -1) {
      switch (option) {
        case 'h':
          hostAddress = optarg;
          break;
        case 'p':
          try {
            publishPort = std::stoi(optarg);
          } catch (...) {
            std::cout << "Invalid value for -p option." << std::endl;
            return 1;
          }

        case 'c':
          try {
            commandConnectionString = std::stoi(optarg);
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
  }

  if (commandConnectionString.empty() || publishConnectionString.empty()) {
    //
    if (hostAddress.empty() || (commandPort == -1) || (publishPort == -1)) {
      std::cout
          << "Operation failed. Please check the host address and port numbers."
          << std::endl;
      return -1;
    }

    commandConnectionString =
        "tcp://" + hostAddress + ":" + std::to_string(commandPort);
    publishConnectionString =
        "tcp://" + hostAddress + ":" + std::to_string(publishPort);
  }
  // Start the command service thread
  cout << "Starting the command service..." << endl;
  std::thread cmd_thread{createRespondService, commandConnectionString.c_str()};

  // Start the publish service thread
  cout << "Starting the publish service..." << endl;
  std::thread pub_thread{createPubService, publishConnectionString.c_str()};

  cout << "Service start completed successfully." << endl;

  while (1) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    cout << "Service is running..." << endl;
  }

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
