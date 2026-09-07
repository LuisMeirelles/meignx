//
// Created by meirelles on 9/7/26.
//

#include "send_params.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "add_param.h"
#include "fcgi.h"

void send_params(const int fd)
{
    uint8_t body[4096] = {0};
    size_t body_length = 0;

    add_param("REQUEST_METHOD", "GET", body, &body_length, sizeof(body));

    if (body_length > UINT16_MAX)
    {
        fprintf(stderr, "FCGI_PARAMS body too large\n");
        exit(4);
    }

    const FCGI_Header header = {
        .version = 1,
        .type = FCGI_PARAMS,
        .requestIdB1 = 0,
        .requestIdB0 = 1,
        .contentLengthB1 = (uint8_t)((body_length >> 8) & 0xFF),
        .contentLengthB0 = (uint8_t)(body_length & 0xFF),
        .paddingLength = 0,
        .reserved = 0,
    };

    const size_t header_size = sizeof(header);
    const size_t record_size = header_size + body_length;

    uint8_t* record = malloc(record_size);

    if (record == NULL)
    {
        perror("malloc");
        exit(errno);
    }

    memcpy(record, &header, header_size);
    memcpy(record + header_size, body, body_length);

    // TODO: send_all
    const ssize_t sent = send(fd, record, record_size, 0);

    free(record);

    if (sent == -1)
    {
        perror("send");
        exit(errno);
    }

    if ((size_t)sent != record_size)
    {
        fprintf(stderr, "partial send: %zd/%zu\n",
                sent, record_size);
    }
}
