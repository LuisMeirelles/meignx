//
// Created by meirelles on 9/7/26.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

void add_param(const char* name, const char* value, uint8_t* buf, size_t* length, int capacity);
