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
static void add_param(const char *name, const char *value, uint8_t *buf, int *length, const int capacity)
{
    uint8_t *end_of_buf = buf + *length;

    const uint8_t name_len = (uint8_t) strlen(name);
    const uint8_t value_len = (uint8_t) strlen(value);

    const int params_size = *length + name_len + value_len + 2;

    if (params_size > capacity)
    {
        dprintf(STDERR_FILENO, "The size of the parameters exceeds the capacity of %d.", capacity);
        exit(1);
    }

    *end_of_buf = name_len;
    end_of_buf++;
    (*length)++;

    *end_of_buf = value_len;
    end_of_buf++;
    (*length)++;

    memcpy(end_of_buf, name, name_len);
    end_of_buf += name_len;
    *length += (int) name_len;

    memcpy(end_of_buf, value, value_len);
    end_of_buf += value_len;
    *length += (int) value_len;
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

    uint8_t body[20] = {0};
    int length = 0;

    add_param("REQUEST_METHOD", "GET", body, &length, 20);
    add_param("SERVER_NAME", "localhost", body, &length, 20);

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
