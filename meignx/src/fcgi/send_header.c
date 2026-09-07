//
// Created by meirelles on 9/7/26.
//

#include "send_header.h"

#include <stdint.h>
#include <unistd.h>

#include "fcgi.h"
#include "lib/send_all.h"

ssize_t send_header(const int fd, const uint8_t type, const size_t body_length)
{
    const FCGI_Header header = {
        .version = 1,
        .type = type,
        .requestIdB1 = 0,
        .requestIdB0 = 1,
        .contentLengthB1 = (uint8_t)((body_length >> 8) & 0xFF),
        .contentLengthB0 = (uint8_t)(body_length & 0xFF),
        .paddingLength = 0,
        .reserved = 0,
    };

    constexpr size_t header_size = sizeof(header);

    const ssize_t header_sent = send_all(fd, &header, header_size, 0);

    if (header_sent == -1)
    {
        return -1;
    }

    return header_sent;
}
