#pragma once

#include "include.h"

#define CODE_LEN 34

enum region {
    REGION_JP = 0,
    REGION_NA = 1,
    REGION_EU = 2
};

bool GenerateCode(char* bitstream, char* dest, enum region region, uint32_t checksum, bool checksum_verify);

void NumToBits(uint32_t num, int output_size, char* dest);

extern const char* bit_values;