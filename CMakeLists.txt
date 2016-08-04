project(viFileManager)
cmake_minimum_required(VERSION 2.8)

add_definitions(-Wall -Wno-unused-but-set-variable -Wno-unused-parameter -std=c++14)
aux_source_directory(src SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
