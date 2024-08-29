#include "include.h"
#include "precompute.h"
#include "generator.h"

#define BITSTREAM_NO_CHECKSUM_LEN 138
#define BITSTREAM_FULL_LEN 170
#define NUM_BLOCKS 17

uint32_t CalculateChecksum(char* bitstream) {
    // Start with 0xFFFFFFFF.
    uint32_t checksum = 0xFFFFFFFF;

    // We have 17 blocks of 8 bits in the bitStream (136 bits).
    for (int i = 16; i >= 0; i--) {
        // Grab 8 bits from the stream and convert it to a number.
        char* bit_ptr = bitstream + i * 8 + 2;
        char substring[8 + 1] = "";
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

int GetResetByte(uint32_t checksum) {
    uint8_t checksum_byte = checksum % 256;
    uint8_t reset_byte = (checksum_byte / 16) + 8 + (checksum_byte % 16);
    // The resetByte must be under 17. If not, the code doesn't use a resetByte.
    return (reset_byte < 17) ? reset_byte : -1;
}

void NumToBits(uint32_t num, int output_size, char* dest) {
    int index = output_size - 1;
    while (index >= 0) {
        dest[index] = (num % 2) + '0';
        num /= 2;
        index--;
    }
}

bool EncryptBitStream(char* bitstream, int checksum_override, bool checksum_verify) {
    uint32_t checksum;
    if (checksum_verify) {
        checksum = CalculateChecksum(bitstream);
        if (checksum % 256 != checksum_override) {
            return false;
        }
    } else {
        checksum = checksum_override;
    }

    char* bit_ptr = bitstream + BITSTREAM_NO_CHECKSUM_LEN;

    // This will contain the 8-bit blocks as numbers (0-255), each representing one byte.
    // The checksum byte is NOT included in these blocks.
    // The first block in the array is the last block in the bitstream (we work backwards).
    uint8_t blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS - 1; i++) {
        bit_ptr -= 8;
        char substring[8 + 1] = "";
        strncpy(substring, bit_ptr, 8);
        int data = strtol(substring, NULL, 2);
        blocks[i] = data;
    }
    bit_ptr -= 10;
    char substring[10 + 1];
    strncpy(substring, bit_ptr, 10);
    blocks[NUM_BLOCKS - 1] = strtol(substring, NULL, 2);

    // Get our encryption entries.
    uint8_t* entries = precompute.encryption_tables.rows[checksum % 256];

    // Figure out the resetByte.
    int reset_byte = GetResetByte(checksum);

    // Do the encryption.
    int enc_idx = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int pos = i * 8;
        if (enc_idx == reset_byte) {
            enc_idx = 0;
        }

        int input_byte = blocks[i];

        // Add the number in the encryption entry to the input byte.
        int result = (input_byte + entries[enc_idx]) & 0xFF;
        result += input_byte & 0x300;
        // printf("pos %d, value %d, encbyte %d, result is %d\n", i, input_byte, entries[enc_idx], result);

        // Update the data in the block.
        blocks[i] = result;

        enc_idx++;
    }

    // Encrypt the input string in place
    char* encryption_ptr = bitstream;
    for (int i = NUM_BLOCKS - 1; i >= 0; i--) {
        if (i == NUM_BLOCKS - 1) {
            NumToBits(blocks[i], 10, encryption_ptr);
            encryption_ptr += 10;
        } else {
            NumToBits(blocks[i], 8, encryption_ptr);
            encryption_ptr += 8;
        }
    }
    
    // Append the checksum
    NumToBits(checksum, 32, encryption_ptr);
    return true;
}

const char* bit_values = "&67NPR89F0+#STXY45MCHJ-K12=%3Q@W";

void BitsToChars(char* bitstream, char* dest) {
    for (int i = 0; i < CODE_LEN; i++) {
        char substring[5 + 1] = "";
        strncpy(substring, bitstream + (CODE_LEN - i - 1) * 5, 5);
        uint8_t num = strtol(substring, NULL, 2);
        if (num >= 0 && num < 32) {
            dest[i] = bit_values[num];
        } else {
            fprintf(stderr, "Bad character value: %d", num);
            exit(EXIT_FAILURE);
        }
    }
}

void ScrambleString(char* dest, uint8_t* byte_swap) {
    char orig[CODE_LEN + 1] = "";
    strncpy(orig, dest, CODE_LEN);

    for (int i = 0; i < CODE_LEN; i++) {
        int target = byte_swap[i];
        dest[target] = orig[i];
    }
}

uint8_t byte_swap_JP[] = {
    0x14, 0x00, 0x13, 0x16, 0x05, 0x12, 0x02, 0x0B,
    0x0C, 0x19, 0x21, 0x0F, 0x08, 0x1D, 0x11, 0x1A,
    0x06, 0x01, 0x17, 0x1C, 0x07, 0x1B, 0x0D, 0x1F,
    0x15, 0x09, 0x1E, 0x0A, 0x20, 0x10, 0x0E, 0x04,
    0x03, 0x18
};

uint8_t byte_swap_NA[] = {
    0x07, 0x1B, 0x0D, 0x1F, 0x15, 0x1A, 0x06, 0x01,
    0x17, 0x1C, 0x09, 0x1E, 0x0A, 0x20, 0x10, 0x21,
    0x0F, 0x08, 0x1D, 0x11, 0x14, 0x00, 0x13, 0x16,
    0x05, 0x12, 0x0E, 0x04, 0x03, 0x18, 0x02, 0x0B,
    0x0C, 0x19
};

uint8_t byte_swap_EU[] = {
    0x0E, 0x04, 0x03, 0x18, 0x09, 0x1E, 0x0A, 0x20,
    0x10, 0x21, 0x14, 0x00, 0x13, 0x16, 0x05, 0x12,
    0x06, 0x01, 0x17, 0x1C, 0x07, 0x1B, 0x0D, 0x1F,
    0x15, 0x1A, 0x02, 0x0B, 0x0C, 0x19, 0x0F, 0x08,
    0x1D, 0x11
};

bool GenerateCode(char* bitstream, char* dest, enum region region, uint32_t checksum, bool checksum_verify) {
    char bitstream_cpy[BITSTREAM_FULL_LEN + 1] = "";
    strncat(bitstream_cpy, bitstream, BITSTREAM_FULL_LEN);

    // Encrypt the code.
    if (!EncryptBitStream(bitstream_cpy, checksum, checksum_verify)) {
        return false;
    };

    // Bitpack it.
    BitsToChars(bitstream_cpy, dest); 

    // Scramble it.
    uint8_t* byte_swap;
    switch(region) {
        case REGION_JP:
            byte_swap = byte_swap_JP;
            break;
        case REGION_NA:
            byte_swap = byte_swap_NA;
            break;
        case REGION_EU:
            byte_swap = byte_swap_EU;
            break;
        default:
            byte_swap = byte_swap_NA;
    }

    ScrambleString(dest, byte_swap);
    return true;
}