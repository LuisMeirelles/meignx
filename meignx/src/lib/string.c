//
// Created by meirelles on 9/19/26.
//

#include "lib/string.h"

#include <string.h>

void explode(char* haystack, const char* needle, char* parts[])
{
    int i = 0;
    char* limit = {nullptr};
    const size_t needle_len = strlen(needle);

    do
    {
        limit = strstr(haystack, needle);

        if (limit != nullptr)
        {
            *limit = '\0';

            parts[i] = haystack;

            haystack = limit + needle_len;

            i++;
        }
        else
        {
            parts[i] = haystack;
        }
    }
    while (limit != nullptr);
}
