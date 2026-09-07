//
// Created by meirelles on 9/7/26.
//

#pragma once

#include <stdint.h>

#include "add_param.h"

#define MAX_BODY_SIZE UINT16_MAX

typedef enum: uint8_t
{
    SEND_PARAMS_OK = 0,
    SEND_PARAMS_PROTOCOL_ERR,
    SEND_PARAMS_NETWORK_ERR,
    SEND_PARAMS_BODY_TOO_LARGE,
} SendParamsTag;

typedef struct
{
    SendParamsTag tag;

    union
    {
        struct
        {
            AddParamResult previous;
        } protocol_error;

        struct
        {
            int sys_errno;
        } network_error;

        struct
        {
            unsigned short calculated_size;
            unsigned short max_size;
        } body_error;
    } payload;
} SendParamsResult;

SendParamsResult send_params(int fd);
