//
// Created by meirelles on 9/16/26.
//

#include "process_stdout.h"

#include <stdlib.h>

#include "lib/string.h"

int process_stdout(char* content, StdoutReponse* stdout_buf)
{
    int i = 0;
    char* http_headers = content;

    // TODO: dynamic header size
    char* headers[3] = {nullptr};

    char** parts = {nullptr};

    explode(http_headers, "\r\n\r\n");

    http_headers = parts[0];
    stdout_buf->body = parts[1];

    explode(http_headers, "\r\n");

    constexpr size_t headers_count = sizeof(headers) / sizeof(headers[0]);

    while (headers[i] != nullptr)
    {
        explode(headers[i], ":");

        char* value = parts[1];

        while (*value == ' ') value++;

        stdout_buf->headers[i].key = parts[0];
        stdout_buf->headers[i].value = value;

        i++;
    }

    return 0;
}
