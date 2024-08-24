#include "include.h"
#include "precompute.h"

int main(int argc, char const *argv[])
{
    GenCRC32Table();
    GenEncryptionTables();

    

    free(precompute.crc32_table);
    free(precompute.encryption_tables.data);
    free(precompute.encryption_tables.rows);
    return 0;
}
