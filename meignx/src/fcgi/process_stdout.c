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

    char* parts[2] = {0};

    explode(http_headers, "\r\n\r\n", parts);

    http_headers = parts[0];
    stdout_buf->body = parts[1];

    explode(http_headers, "\r\n", headers);

    constexpr size_t headers_count = sizeof(headers) / sizeof(headers[0]);

    for (i = 0; i < headers_count; i++)
    {
        explode(headers[i], ":", parts);

        char* value = parts[1];

        while (*value == ' ') value++;

        stdout_buf->headers[i].key = parts[0];
        stdout_buf->headers[i].value = value;
    }

    return 0;
}
