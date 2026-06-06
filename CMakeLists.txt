cmake_minimum_required(VERSION 3.31)
project(scratch_graphics_for_fun)

set(CMAKE_CXX_STANDARD 20)

add_executable(scratch_graphics_for_fun main.cpp
        kernel_src.c)
