cmake_minimum_required(VERSION 4.2.3)
project(meignx C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(meignx
        include/fcgi.h
        src/main.c)
