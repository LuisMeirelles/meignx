//
// Created by meirelles on 9/16/26.
//

#include "../../include/process_stdout.h"

#include <stdlib.h>
#include <string.h>

int process_stdout(char* content, StdoutReponse* stdout_buf)
{
    int i = 0;
    char* http_headers = content;
    char* headers[64] = {nullptr};
    size_t headers_count = 0;

    char* end_headers = strstr(http_headers, "\r\n\r\n");

    *end_headers = '\0';

    stdout_buf->body = end_headers + 4;

    while (1)
    {
        char* tok = strtok(http_headers, "\r\n");

        if (tok == nullptr)
        {
            break;
        }

        headers[headers_count] = tok;

        http_headers = nullptr;
        headers_count++;
    }

    for (i = 0; i < headers_count; i++)
    {
        char* delim = strchr(headers[i], ':');

        if (delim != nullptr)
        {
            *delim = '\0';

            char* value = delim + 1;

            while (*value == ' ') value++;

            stdout_buf->headers[i].key = headers[i];
            stdout_buf->headers[i].value = value;
        }
    }

    return 0;
}
