cmake_minimum_required(VERSION 3.10)
project(architecture-2-main)

set(CMAKE_CXX_STANDARD 11)

#исходные файлы
add_executable(server server.c++)
add_executable(client client.c++)

add_executable(server_sock server.c++)
add_executable(client_sock client.c++)
