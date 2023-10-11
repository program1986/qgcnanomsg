cmake_minimum_required(VERSION 3.0.0)
project(nanotest VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

include(CTest)
enable_testing()

# namomsg support
include_directories(lib/nanomsg/include)
link_directories(lib/nanomsg/lib)

# jsoncpp support
include_directories(lib/jsoncpp/include)
link_directories(lib/jsoncpp/lib)


add_executable(nanoserver nanomsg_server.cpp)
add_executable(nanoclient nanomsg_client.cpp)
target_link_libraries(nanoclient nanomsg jsoncpp)
target_link_libraries(nanoserver nanomsg jsoncpp)

#example for pair

add_executable(pair_sender example_src/pair/sender.cpp)
add_executable(pair_receiver example_src/pair/receiver.cpp)
target_link_libraries(pair_sender nanomsg jsoncpp)
target_link_libraries(pair_receiver nanomsg jsoncpp)

# example for request
add_executable(requester example_src/request/requester.cpp)
add_executable(responder example_src/request/responder.cpp)
target_link_libraries(responder nanomsg jsoncpp)
target_link_libraries(requester nanomsg jsoncpp)

# example for bus

add_executable(endpoint1 example_src/bus/endpoint1.cpp)
add_executable(endpoint2 example_src/bus/endpoint2.cpp)
target_link_libraries(endpoint1 nanomsg jsoncpp)
target_link_libraries(endpoint2 nanomsg jsoncpp)

# exmple for pubsub
add_executable(publisher example_src/pubsub/publisher.cpp)
add_executable(subscriber example_src/pubsub/subscriber.cpp)
target_link_libraries(publisher nanomsg jsoncpp)
target_link_libraries(subscriber nanomsg jsoncpp)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
