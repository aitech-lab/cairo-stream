cmake_minimum_required(VERSION 3.0)

add_executable(drawing drawing.c)
target_link_libraries(drawing cairo pthread)
