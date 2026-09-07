//
// Created by meirelles on 9/7/26.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

ssize_t send_header(int fd, uint8_t type, size_t body_length);
