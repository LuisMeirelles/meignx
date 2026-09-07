//
// Created by meirelles on 9/7/26.
//

#include "add_param.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * FastCGI transmits a name-value pair as the length of the name, followed by the length of the value,
 * followed by the name, followed by the value. Lengths of 127 bytes and less can be encoded in one byte,
 * while longer lengths are always encoded in four bytes
 *
 * @see https://fastcgi-archives.github.io/FastCGI_Specification.html#34-name-value-pairs
 */
void add_param(const char* name, const char* value, uint8_t* buf, size_t* length, const int capacity)
{
    uint8_t* end_of_buf = buf + *length;

    const size_t name_len = strlen(name);

    if (name_len > 0x7FFFFFFF)
    {
        fprintf(stderr, "The size of the name exceeds the maximum size");
        exit(1);
    }

    const size_t value_len = strlen(value);

    if (value_len > 0x7FFFFFFF)
    {
        fprintf(stderr, "The size of the value exceeds the maximum size");
        exit(2);
    }

    const size_t name_len_size = name_len < 128 ? 1 : 4;
    const size_t value_len_size = value_len < 128 ? 1 : 4;

    const size_t param_size = name_len
        + value_len
        + name_len_size
        + value_len_size;

    const unsigned long required_size = *length + param_size;

    if (required_size > capacity)
    {
        fprintf(stderr, "The size of the parameters exceeds the capacity of %d.", capacity);
        exit(3);
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

    *length += (int)name_len_size;

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

    *length += (int)value_len_size;

    memcpy(end_of_buf, name, name_len);
    end_of_buf += name_len;
    *length += (int)name_len;

    memcpy(end_of_buf, value, value_len);
    end_of_buf += value_len;
    *length += (int)value_len;
}
