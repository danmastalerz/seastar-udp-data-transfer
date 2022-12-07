cmake_minimum_required(VERSION 3.23)
project(seastar_send_receive_data)

# set clang as compiler
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 20)

find_package(Seastar REQUIRED)

add_executable(seastar_send_receive_data main.cpp)
add_executable(seastar_send_receive_data2 main2.cpp)
target_link_libraries(seastar_send_receive_data Seastar::seastar)
target_link_libraries(seastar_send_receive_data2 Seastar::seastar)
