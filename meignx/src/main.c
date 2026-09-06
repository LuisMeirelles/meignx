#include <errno.h>
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
static void add_param(const char* name, const char* value, uint8_t* buf, int* length, const int capacity)
{
    uint8_t* end_of_buf = buf + *length;

    const size_t name_len = strlen(name);

    if (name_len > 0x7FFFFFFF)
    {
        dprintf(STDERR_FILENO, "The size of the name exceeds the maximum size");
        exit(1);
    }

    const size_t value_len = strlen(value);

    if (value_len > 0x7FFFFFFF)
    {
        dprintf(STDERR_FILENO, "The size of the value exceeds the maximum size");
        exit(1);
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
        dprintf(STDERR_FILENO, "The size of the parameters exceeds the capacity of %d.", capacity);
        exit(2);
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
    typedef struct
    {
        unsigned char nameLengthB0;
        unsigned char valueLengthB0;
        unsigned char nameData[14];
        unsigned char valueData[3];
    } FCGI_NameValuePair;

    typedef struct
    {
        FCGI_Header header;
        FCGI_NameValuePair body;
    } FCGI_ParamsRecord;

    const FCGI_ParamsRecord record = {
        .header = {
            .version = 1,
            .type = FCGI_PARAMS,
            .requestIdB1 = 0,
            .requestIdB0 = 1,
            .contentLengthB1 = 0,
            .contentLengthB0 = 19,
            .paddingLength = 0,
            .reserved = 0,
        },
        .body = {
            .nameLengthB0 = 14,
            .valueLengthB0 = 3,
            .nameData = "REQUEST_METHOD",
            .valueData = "GET",
        },
    };

    uint8_t body[4096] = {0};
    int length = 0;

    add_param("REQUEST_METHOD", "GET", body, &length, 4096);

    const ssize_t success = send(fd, &record, sizeof(record), 0);

    if (success == -1)
    {
        perror("send");
        exit(errno);
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
