//
// Created by meirelles on 9/7/26.
//

#include "send_params.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

#include "add_param.h"
#include "fcgi.h"
#include "lib/send_all.h"

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

    constexpr size_t header_size = sizeof(header);

    const ssize_t header_sent = send_all(fd, &header, header_size, 0);

    if (header_sent == -1)
    {
        make_network_error(errno);
    }

    const ssize_t body_sent = send_all(fd, &body, body_length, 0);

    if (body_sent == -1)
    {
        make_network_error(errno);
    }

    return (SendParamsResult){.tag = SEND_PARAMS_OK};
}
