cmake_minimum_required(VERSION 4.2.3)
project(meignx C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(meignx
        include/fcgi.h
        src/main.c
        src/fcgi/begin_request.c
        include/begin_request.h
        src/fcgi/add_param.c
        include/add_param.h
        src/use_cases/send_params.c
        include/send_params.h)
