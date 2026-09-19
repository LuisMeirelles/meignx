//
// Created by meirelles on 9/16/26.
//

#pragma once

#include "fcgi.h"
#include "lib/params.h"

typedef struct
{
    char* body;
    Param headers[64];
} StdoutReponse;

int process_stdout(char* content, StdoutReponse* stdout_buf);
