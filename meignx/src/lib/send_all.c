//
// Created by meirelles on 9/7/26.
//

#include "lib/send_all.h"

#include <stdint.h>
#include <sys/socket.h>

ssize_t send_all(const int fd, const void* buf, const size_t n, const int flags)
{
    size_t total_sent = 0;

    do
    {
        const ssize_t sent = send(fd, (const uint8_t*)buf + total_sent, n - total_sent, flags);

        if (sent == -1)
        {
            return -1;
        }

        total_sent += (size_t)sent;
    }
    while (n != total_sent);

    return (ssize_t)total_sent;
}
