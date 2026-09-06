#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
