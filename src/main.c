#include "include.h"
#include "precompute.h"
#include "generator.h"

int main(int argc, char const *argv[])
{
    GenCRC32Table();
    GenEncryptionTables();
    // Check that input string is 136 and convert it to 170 length, leave checksum as zeros
                     //0000000000000000000001000000010000010010010011111000000000000000000000110110100000001101101000000000000000000000100000000001000000000100
    char string[] = "00000000000000000000000100000001000001001001001111100000000000000000000011011010000000110110100000000000000000000010000000000100000000010000000000000000000000000000000000";
    char dest[35] = "";
    GenerateCode(string, dest, REGION_EU); 
    printf("%s\n", dest);
    free(precompute.crc32_table);
    free(precompute.encryption_tables.data);
    free(precompute.encryption_tables.rows);
    return 0;
}
