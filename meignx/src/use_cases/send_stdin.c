//
// Created by meirelles on 9/7/26.
//

#include "send_stdin.h"

#include "fcgi.h"
#include "send_header.h"

void send_stdin(const int fd)
{
    send_header(fd, FCGI_STDIN, 0);
}
