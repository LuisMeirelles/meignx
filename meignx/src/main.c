#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "begin_request.h"
#include "send_params.h"

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

    send_params(fd);

    printf("funcionou");

    close(fd);

    return 0;
}
