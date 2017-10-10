cmake_minimum_required(VERSION 3.7)
project(chatProject)

add_subdirectory(path/to/client)
add_subdirectory(path/to/server)

add_custom_target(chatProject)
add_dependencies(chatProject client server)