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
#include "send_header.h"
#include "lib/send_all.h"

#define BODY_BUF_SIZE 148

typedef struct
{
    char* key;
    char* value;
} Param;

static SendParamsResult make_protocol_error(const AddParamResult error)
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

static SendParamsResult send_fcgi_params_request(const int fd, const Param params[], const size_t params_len)
{
    uint8_t body[BODY_BUF_SIZE] = {0};
    size_t body_length = 0;

    for (int i = 0; i < params_len; ++i)
    {
        const AddParamResult add_param_result = add_param(
            params[i].key,
            params[i].value,
            body,
            &body_length,
            sizeof(body)
        );

        if (add_param_result.tag != ADD_PARAM_OK)
        {
            return make_protocol_error(add_param_result);
        }
    }

    if (body_length > MAX_BODY_SIZE)
    {
        return make_body_too_large_error(body_length);
    }

    const ssize_t header_sent = send_header(fd, FCGI_PARAMS, body_length);

    if (header_sent == -1)
    {
        return make_network_error(errno);
    }

    const ssize_t body_sent = send_all(fd, &body, body_length, 0);

    if (body_sent == -1)
    {
        make_network_error(errno);
    }

    return (SendParamsResult){.tag = SEND_PARAMS_OK};
}

SendParamsResult send_params(const int fd)
{
    const Param params[] = {
        {.key = "REQUEST_METHOD", .value = "GET"},
        {.key = "SCRIPT_FILENAME", .value = "/var/www/html/index.php"},
        {.key = "SCRIPT_NAME", .value = "/index.php"},
        {.key = "REQUEST_URI", .value = "/index.php"},
        {.key = "QUERY_STRING", .value = ""},
        {.key = "SERVER_PROTOCOL", .value = "HTTP/1.1"},
    };

    constexpr size_t count = sizeof(params) / sizeof(Param);

    const SendParamsResult send_params_result = send_fcgi_params_request(fd, params, count);

    if (send_params_result.tag != SEND_PARAMS_OK)
    {
        return send_params_result;
    }

    return send_fcgi_params_request(fd, nullptr, 0);
}
