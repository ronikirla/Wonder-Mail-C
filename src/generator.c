#include "include.h"
#include "precompute.h"

#define BITSTREAM_LEN 138
#define NUM_BLOCKS 17

uint32_t CalculateChecksum(char* bitstream) {
    // Start with 0xFFFFFFFF.
    uint32_t checksum = 0xFFFFFFFF;

    // We have 17 blocks of 8 bits in the bitStream (136 bits).
    for (int i = 16; i >= 0; i--) {
        // Grab 8 bits from the stream and convert it to a number.
        char* bit_ptr = bitstream + i * 8 + 2;
        char substring[8];
        strncpy(substring, bit_ptr, 8);
        uint8_t num = strtol(substring, NULL, 2);
        
        // Grab a entry from the data table. The entry gotten is equal to 
        uint32_t entry = precompute.crc32_table[(checksum ^ num) & 0xFF];
        
        // The entry is NOT'ed with our current checksum rsl'd 8 times. The result of this will be the new checksum
        // for this round.
        checksum = (checksum >> 8) ^ entry;
    }
    
    // Our final checksum is NOT'ed with 0xFFFFFFFF.
    checksum = checksum ^ 0xFFFFFFFF;
    
    return checksum;
}

uint8_t GetResetByte(uint32_t checksum) {
    uint8_t checksumByte = checksum % 256;
    uint8_t resetByte = (checksumByte / 16) + 8 + (checksumByte % 16);
    // The resetByte must be under 17. If not, the code doesn't use a resetByte.
    return (resetByte < 17) ? resetByte : -1;
}

char* EncryptBitStream(char* bitstream) {
    char* bit_ptr = bitstream + BITSTREAM_LEN;

    // This will contain the 8-bit blocks as numbers (0-255), each representing one byte.
    // The checksum byte is NOT included in these blocks.
    // The first block in the array is the last block in the bitstream (we work backwards).
    int blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS - 1; i++) {
        bit_ptr -= 8;
        char substring[8];
        strncpy(substring, bit_ptr, 8);
        int data = strtol(substring, NULL, 2);
        blocks[i] = data;
        i++;
    }
    bit_ptr -= 2;
    char substring[10];
    strncpy(substring, bit_ptr, 10);
    blocks[NUM_BLOCKS - 1] = strtol(substring, NULL, 2);

    uint32_t checksum = CalculateChecksum(bitstream);

    // Get our encryption entries.
    uint8_t* entries = precompute.encryption_tables.rows[checksum % 256];

    // Figure out the resetByte.
    uint8_t resetByte = GetResetByte(checksum);

    // Do the encryption.

}

char* GenerateCode(char* bitstream) {

}