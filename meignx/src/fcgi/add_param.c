//
// Created by meirelles on 9/7/26.
//

#include "add_param.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static AddParamResult make_length_too_large_error(const FCGI_ParamComponent component)
{
    return (AddParamResult){
        .tag = ADD_PARAM_LENGTH_TOO_LARGE,
        .payload.length_error = {
            .max_size = FCGI_PARAM_MAX_SIZE,
            .component = component,
        }
    };
}

static AddParamResult make_buffer_too_small_error(const size_t min_buf_size, const size_t given_capacity)
{
    return (AddParamResult){
        .tag = ADD_PARAM_BUFFER_TOO_SMALL,
        .payload.buffer_error = {
            .min_buf_size = min_buf_size,
            .given_capacity = given_capacity,
        }
    };
}

/**
 * FastCGI transmits a name-value pair as the length of the name, followed by the length of the value,
 * followed by the name, followed by the value. Lengths of 127 bytes and less can be encoded in one byte,
 * while longer lengths are always encoded in four bytes
 *
 * @see https://fastcgi-archives.github.io/FastCGI_Specification.html#34-name-value-pairs
 */
AddParamResult add_param(const char* name, const char* value, uint8_t* buf, size_t* length, const size_t capacity)
{
    uint8_t* end_of_buf = buf + *length;

    const size_t name_len = strlen(name);

    if (name_len > FCGI_PARAM_MAX_SIZE)
    {
        return make_length_too_large_error(FCGI_COMPONENT_NAME);
    }

    const size_t value_len = strlen(value);

    if (value_len > FCGI_PARAM_MAX_SIZE)
    {
        return make_length_too_large_error(FCGI_COMPONENT_VALUE);
    }

    const size_t name_len_size = name_len < 128 ? 1 : 4;
    const size_t value_len_size = value_len < 128 ? 1 : 4;

    const size_t param_size = name_len
        + value_len
        + name_len_size
        + value_len_size;

    const size_t required_size = *length + param_size;

    if (required_size > capacity)
    {
        return make_buffer_too_small_error(required_size, capacity);
    }

    if (name_len < 128)
    {
        *end_of_buf++ = name_len;
    }
    else
    {
        *end_of_buf++ = ((name_len >> 24) & 0x7F) | 0x80;
        *end_of_buf++ = (name_len >> 16) & 0xFF;
        *end_of_buf++ = (name_len >> 8) & 0xFF;
        *end_of_buf++ = name_len & 0xFF;
    }

    if (value_len < 128)
    {
        *end_of_buf++ = value_len;
    }
    else
    {
        *end_of_buf++ = ((value_len >> 24) & 0x7F) | 0x80;
        *end_of_buf++ = (value_len >> 16) & 0xFF;
        *end_of_buf++ = (value_len >> 8) & 0xFF;
        *end_of_buf++ = value_len & 0xFF;
    }

    memcpy(end_of_buf, name, name_len);
    end_of_buf += name_len;

    memcpy(end_of_buf, value, value_len);
    end_of_buf += value_len;

    *length = required_size;

    return (AddParamResult){.tag = ADD_PARAM_OK};
}
