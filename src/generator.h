#pragma once

enum region {
    REGION_JP = 0,
    REGION_NA = 1,
    REGION_EU = 2
};

void GenerateCode(char* bitstream, char* dest, enum region region);