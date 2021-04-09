cmake_minimum_required(VERSION 3.14)
project(projekt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets)
add_executable(${PROJECT_NAME} main.cpp src/qt/mainwindow.cpp src/Mqttclient.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC src src/qt)

# Include QT from system
find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${REQUIRED_LIBS_QUALIFIED})
find_package(PahoMqttCpp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PahoMqttCpp::paho-mqttpp3-static)