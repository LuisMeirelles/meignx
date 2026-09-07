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

#define BODY_BUF_SIZE 19

static SendParamsResult make_validation_error(const AddParamResult error)
{
    return (SendParamsResult){
        .tag = SEND_PARAMS_PROTOCOL_ERR,
        .payload.protocol_error.previous = error,
    };
}

static SendParamsResult make_body_too_large_error(const unsigned short calculated_size)
{
    return (SendParamsResult){
        .tag = SEND_PARAMS_BODY_TOO_LARGE,
        .payload.body_error = {
            .calculated_size = calculated_size,
            .max_size = MAX_BODY_SIZE,
        }
    };
}

static SendParamsResult make_network_error(const int sys_errno)
{
    return (SendParamsResult){
        .tag = SEND_PARAMS_NETWORK_ERR,
        .payload.network_error.sys_errno = sys_errno,
    };
}

SendParamsResult send_params(const int fd)
{
    uint8_t body[BODY_BUF_SIZE] = {0};
    size_t body_length = 0;

    const AddParamResult add_param_result = add_param("REQUEST_METHOD", "GET", body, &body_length, sizeof(body));

    if (add_param_result.tag != ADD_PARAM_OK)
    {
        return make_validation_error(add_param_result);
    }

    if (body_length > MAX_BODY_SIZE)
    {
        return make_body_too_large_error(body_length);
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

    // TODO: use send directly without malloc and memcpy
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
        make_network_error(errno);
    }

    return (SendParamsResult){.tag = SEND_PARAMS_OK};
}
