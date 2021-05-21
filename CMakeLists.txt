cmake_minimum_required(VERSION 3.14)
project(mqtt-explorer-fit)

# PahoMqttCpp does to work on CPP 17+
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets)
add_executable(${PROJECT_NAME} src/main.cpp src/qt/dashboarditem.cpp src/qt/mainwindow.cpp src/Mqttclient.cpp src/qt/TopicHistoryItemDelegate.cpp
		src/qt/TopicHistoryItemDelegate.h src/qt/messageviewdialog.cpp src/qt/messageviewwidget.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC src src/qt)

add_executable(trafficSimulator src/trafficSimulator.cpp)
target_link_libraries(trafficSimulator PRIVATE PahoMqttCpp::paho-mqttpp3-static)

# Include QT from system
find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${REQUIRED_LIBS_QUALIFIED})
find_package(PahoMqttCpp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PahoMqttCpp::paho-mqttpp3-static)

# Doxygen
option(BUILD_DOC "Build documentation" ON)
find_package(Doxygen)
if (DOXYGEN_FOUND)
	add_custom_target( doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
		COMMAND ${DOXYGEN_EXECUTABLE} simDoxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating project documentation with Doxygen"
		VERBATIM)
else (DOXYGEN_FOUND)
	message("Doxygen has to be installed to generate the documentation")
endif (DOXYGEN_FOUND)
