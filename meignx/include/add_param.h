//
// Created by meirelles on 9/7/26.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

#define FCGI_PARAM_MAX_SIZE 0x7FFFFFFF

typedef enum: uint8_t
{
    ADD_PARAM_OK = 0,
    ADD_PARAM_LENGTH_TOO_LARGE,
    ADD_PARAM_BUFFER_TOO_SMALL,
} AddParamStatusTag;

typedef enum
{
    FCGI_COMPONENT_NAME,
    FCGI_COMPONENT_VALUE,
} FCGI_ParamComponent;

typedef struct
{
    AddParamStatusTag tag;

    union
    {
        struct
        {
            // ok returning constant because it's using padding space
            size_t max_size;
            FCGI_ParamComponent component;
        } length_error;

        struct
        {
            size_t min_buf_size;
            size_t given_capacity;
        } buffer_error;
    } payload;
} AddParamResult;

AddParamResult add_param(const char* name, const char* value, uint8_t* buf, size_t* length, size_t capacity);
