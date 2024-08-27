#pragma once

enum mission_type {
    MISSION_TYPE_PROSPECT = 1,
    MISSION_TYPE_NOMRAL = 2,
    MISSION_TYPE_TREASURE_HUNT = 3
};

char* GenerateOptimizedCode(char* bitstream, enum region region, enum mission_type mission_type);