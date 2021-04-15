cmake_minimum_required(VERSION 3.14)
project(test)

# Getting multiple definition of DFLT_QOS and DFLT_RETAINED when using C++17
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} test.cpp)

find_package(PahoMqttCpp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PahoMqttCpp::paho-mqttpp3-static)
