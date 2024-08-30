#include "generator.h"
#include "include.h"
#include "optimize.h"
#include "valid_lists.h"

#define MAX_ASCII 128 

#define POS_NB_SF           5
#define POS_FLAVOR_TEXT_MSB 32
#define POS_FLAVOR_TEXT_LSB 55
#define POS_TARGET          104
#define POS_CLIENT          115

#define BITS_NB_SF              11
#define BITS_FLAVOR_TEXT_MSB    3
#define BITS_FLAVOR_TEXT_FULL   24
#define BITS_FLAVOR_TEXT_LSB    1
#define BITS_TARGET_CLIENT      11

struct evaluation {
    int repeats;
    float distance;
};

struct coords {
    int x;
    int y;
};

struct coords GetKeyCoords(char key) {
    switch (key) {
        case 'C':
            return (struct coords) {2, 0};
        case 'F':
            return (struct coords) {5, 0};
        case 'H':
            return (struct coords) {7, 0};
        case 'J':
            return (struct coords) {9, 0};
        case 'K':
            return (struct coords) {10, 0};
        case 'M':
            return (struct coords) {12, 0};
        case 'N':
            return (struct coords) {0, 1};
        case 'P':
            return (struct coords) {2, 1};
        case 'Q':
            return (struct coords) {3, 1};
        case 'R':
            return (struct coords) {4, 1};
        case 'S':
            return (struct coords) {5, 1};
        case 'T':
            return (struct coords) {6, 1};
        case 'W':
            return (struct coords) {9, 1};
        case 'X':
            return (struct coords) {10, 1};
        case 'Y':
            return (struct coords) {11, 1};
        case '0':
            return (struct coords) {0, 2};
        case '1':
            return (struct coords) {1, 2};
        case '2':
            return (struct coords) {2, 2};
        case '3':
            return (struct coords) {3, 2};
        case '4':
            return (struct coords) {4, 2};
        case '5':
            return (struct coords) {5, 2};
        case '6':
            return (struct coords) {6, 2};
        case '7':
            return (struct coords) {7, 2};
        case '8':
            return (struct coords) {8, 2};
        case '9':
            return (struct coords) {9, 2};
        case '@':
            return (struct coords) {10, 2};
        case '&':
            return (struct coords) {12, 2};
        case '-':
            return (struct coords) {0, 3};
        case '#':
            return (struct coords) {2, 3};
        case '%':
            return (struct coords) {4, 3};
        case '+':
            return (struct coords) {8, 3};
        case '=':
            return (struct coords) {10, 3};
        default:
            // Return a default value if character is not recognized
            return (struct coords) {-1, -1};
    }
}

struct evaluation EvaluateCode(char* code, int best_c) {
    int c = 0;
    float d = 0;
    for (int i = 1; i < CODE_LEN; i++) {
        // Count repeated chars
        if (code[i] == code[i - 1]) {
            c++;
        }
    }
    if (c >= best_c) {
        for (int i = 0; i < CODE_LEN; i++) {
            // Calculate distance between chars
            struct coords key0 = GetKeyCoords(code[i - 1]);
            struct coords key1 = GetKeyCoords(code[i    ]);

            int x0 = key0.x;
            int y0 = key0.y;
            int x1 = key1.x;
            int y1 = key1.y;

            if (x0 == -1 || y0 == -1 || x1 == -1 || y1 == -1) {
                continue;
            }

            d += sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));            
        }
    } else {
        d = FLT_MAX;
    }
    return (struct evaluation) { c, d };
}

/* not needed if we can just set everything perfectly void* SetUnks(char* code, enum region region, enum mission_type mission_type) {
    int unks[CODE_LEN];
    int unks_len = 0;

    

    for (int i = 0; i < unks_len; i++) {
        code[unks[i]] = '?';
    }
}*/

char* GenerateOptimizedCode(char* bitstream, enum region region, enum mission_type mission_type) {
    // Initialize the reverse lookup table with -1 (indicating character not found)
    int reverse_lookup[MAX_ASCII];
    for (int i = 0; i < MAX_ASCII; i++) {
        reverse_lookup[i] = -1;
    }

    // Fill the reverse lookup table with indices of characters in bit_values
    for (int i = 0; i < strlen(bit_values); i++) {
        reverse_lookup[(int) bit_values[i]] = i;
    }


    char* best_code = malloc(sizeof (char[CODE_LEN + 1]));
    char current_code[CODE_LEN + 1] = "";

    struct evaluation best_evaluation = {0, FLT_MAX};

    // Performance critical code so indent level billion is acceptable COPIUM
    switch (mission_type) {
        case MISSION_TYPE_NOMRAL:
            for (int client_idx = 0; client_idx < CLIENTS_LEN; client_idx++) {
                printf("Client: %d / %d\n", client_idx, CLIENTS_LEN);
                NumToBits(clients[client_idx], BITS_TARGET_CLIENT, bitstream + POS_TARGET);
                NumToBits(clients[client_idx], BITS_TARGET_CLIENT, bitstream + POS_CLIENT);
                for (int flavor_text_msb = 0; flavor_text_msb < 8; flavor_text_msb++) {
                    NumToBits(flavor_text_msb, BITS_FLAVOR_TEXT_MSB, bitstream + POS_FLAVOR_TEXT_MSB);
                    for (int flavor_text_lsb = 0; flavor_text_lsb < 2; flavor_text_lsb++) {
                        NumToBits(flavor_text_lsb, BITS_FLAVOR_TEXT_LSB, bitstream + POS_FLAVOR_TEXT_LSB);
                        for (int nb_sf = 0; nb_sf <= 0x7FF; nb_sf++) {
                            NumToBits(nb_sf, BITS_NB_SF, bitstream + POS_NB_SF);
                            for (uint32_t checksum = 0; checksum <= 0xFF; checksum++) {
                                GenerateCode(bitstream, current_code, region, checksum, false);
                                // Switch case region
                                int characters_to_change[] = {31, 21, 26, 2, 17};
                                int neighbors_to_copy[] = {32, 22, 27, 1, 18};
                                int offsets[] = {
                                    POS_FLAVOR_TEXT_LSB - 5 * 1,
                                    POS_FLAVOR_TEXT_LSB - 5 * 2,
                                    POS_FLAVOR_TEXT_LSB - 5 * 3,
                                    POS_FLAVOR_TEXT_LSB - 5 * 4,
                                    0
                                };
                                for (int i = 0; i < 5; i++) {
                                    int character_to_change = characters_to_change[i];
                                    int neighbor_to_copy = neighbors_to_copy[i];

                                    char* bit_ptr = bitstream + offsets[i];
                                    char substring[5 + 1] = "";
                                    strncpy(substring, bit_ptr, 5);
                                    int num = strtol(substring, NULL, 2);

                                    int encrypted_num = reverse_lookup[current_code[character_to_change]];
                                    int encryption_value = encrypted_num - num;

                                    int target_num = reverse_lookup[current_code[neighbor_to_copy]];
                                    int target_num_decrypted = (target_num - encryption_value + 32) % 32;

                                    NumToBits(target_num_decrypted, 5, bit_ptr);
                                }

                                if (!GenerateCode(bitstream, current_code, region, checksum, true)) {
                                    continue;
                                }

                                struct evaluation result = EvaluateCode(current_code, best_evaluation.repeats);
                                if  (result.repeats > best_evaluation.repeats || 
                                    (result.repeats == best_evaluation.repeats &&
                                        result.distance < best_evaluation.repeats))
                                {
                                    strcpy(best_code, current_code);
                                    printf("%s\n", bitstream);
                                    printf("%s\n", current_code);
                                    best_evaluation = result;
                                }
                            }
                        }
                    }
                }
            }
            break;
    }

    return best_code;
}