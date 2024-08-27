#pragma once

#include "include.h"

#define CODE_LEN 34

enum region {
    REGION_JP = 0,
    REGION_NA = 1,
    REGION_EU = 2
};

void GenerateCode(char* bitstream, char* dest, enum region region);

void NumToBits(uint32_t num, int output_size, char* dest);