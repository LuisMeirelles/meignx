#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "fcgi.h"

static void begin_request(const int fd)
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

/**
 * FastCGI transmits a name-value pair as the length of the name, followed by the length of the value,
 * followed by the name, followed by the value. Lengths of 127 bytes and less can be encoded in one byte,
 * while longer lengths are always encoded in four bytes
 *
 * @see https://fastcgi-archives.github.io/FastCGI_Specification.html#34-name-value-pairs
 */
static void add_param(const char* name, const char* value, uint8_t* buf, size_t* length, const int capacity)
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

static void send_get_method(const int fd)
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

int main()
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(9000),
        .sin_addr = {
            .s_addr = 0x220A8C0,
        },
    };

    const int error_connect = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

    if (error_connect != 0)
    {
        perror("connect");
        return errno;
    }

    begin_request(fd);

    send_get_method(fd);

    printf("funcionou");

    close(fd);

    return 0;
}
