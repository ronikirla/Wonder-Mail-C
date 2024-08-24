#pragma once

#include "include.h"

struct encryption_tables {
    uint8_t* data;
    uint8_t** rows;
};

struct precompute {
    uint32_t* crc32_table;
    struct encryption_tables encryption_tables;
};

extern struct precompute precompute;

void GenCRC32Table(void);

void GenEncryptionTables(void);