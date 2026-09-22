//
// Created by meirelles on 9/19/26.
//

#include "lib/string.h"

#include <stdlib.h>
#include <string.h>

char** explode(char* haystack, const char* needle)
{
    int i = 0;
    char* limit = {nullptr};
    const size_t needle_len = strlen(needle);

    int count = 1;

    const char* p = haystack;

    while ((p = strstr(p, needle)) != nullptr)
    {
        count++;
        p += needle_len;
    }

    char** parts = malloc(count * sizeof(char*));

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
            parts[i + 1] = nullptr;
        }
    }
    while (limit != nullptr);
}
