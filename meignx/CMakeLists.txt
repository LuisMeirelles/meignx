cmake_minimum_required(VERSION 3.20)

project(meignx C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(meignx
        src/main.c
        src/fcgi/begin_request.c
        src/fcgi/add_param.c
        src/use_cases/send_params.c
        src/lib/send_all.c
        src/fcgi/send_header.c
)

target_include_directories(meignx PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)