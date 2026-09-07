//
// Created by meirelles on 9/7/26.
//

#pragma once

#include <unistd.h>

ssize_t send_all(int fd, const void *buf, size_t n, int flags);
