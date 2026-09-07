//
// Created by meirelles on 9/7/26.
//

#include "begin_request.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "fcgi.h"

void begin_request(const int fd)
{
    const FCGI_BeginRequestRecord record = {
        .header = {
            .version = 1,
            .type = FCGI_BEGIN_REQUEST,
            .requestIdB1 = 0,
            .requestIdB0 = 1,
            .contentLengthB1 = 0,
            .contentLengthB0 = 8,
            .paddingLength = 0,
            .reserved = 0,
        },
        .body = {
            .roleB1 = FCGI_RESPONDER >> 8,
            .roleB0 = FCGI_RESPONDER & 0xFF,
            .flags = 0,
            .reserved = 0,
        }
    };

    const ssize_t success = send(fd, &record, sizeof(record), 0);

    if (success == -1)
    {
        perror("send");
        exit(errno);
    }
}
