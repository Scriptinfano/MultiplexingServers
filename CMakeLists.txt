cmake_minimum_required(VERSION 3.5.1)
project(NETWORKPROGRAM VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall") #设定编译参数
set(CMAKE_BUILD_TYPE Debug)
include_directories(${CMAKE_SOURCE_DIR}/include) #定义头文件的位置
file(GLOB SRC_FILES src/*.c) #定义源文件的位置
add_executable(main main.c ${SRC_FILES})
