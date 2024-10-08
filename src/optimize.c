#include "generator.h"
#include "include.h"
#include "optimize.h"
#include "valid_lists.h"

#define MAX_ASCII 128 

#define POS_FLAVOR_TEXT_MSB 32
#define POS_FLAVOR_TEXT_LSB 55
#define POS_TARGET          104
#define POS_CLIENT          115

#define BITS_NB_SF              16
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

struct code_details {
    char code[CODE_LEN + 1];
    struct evaluation eval;
};

int compare_codes(const void* a, const void* b) {
    const struct code_details* aa = a;
    const struct code_details* bb = b;
    if (aa->eval.repeats > bb->eval.repeats) {
        return -1;
    } else if (aa->eval.repeats < bb->eval.repeats) {
        return 1;
    } else {
        if (aa->eval.distance < bb->eval.distance) {
            return -1;
        } else if (aa->eval.distance > bb->eval.distance) {
            return 1;
        } else {
            return 0;
        }
    }
}

char* GenerateOptimizedCode(char* bitstream, enum region region, enum mission_type mission_type) {
    // Reverse lookup table for finding the bit value of a given character
    int reverse_lookup[MAX_ASCII];
    for (int i = 0; i < MAX_ASCII; i++) {
        reverse_lookup[i] = -1;
    }

    for (uint32_t i = 0; i < strlen(bit_values); i++) {
        reverse_lookup[(int) bit_values[i]] = i;
    }

    struct code_details best_codes[CLIENTS_LEN];
    int iterations = 0;

    // First brute force parts of the code, including all the bits that don't make 
    // up full characters by themselves (since it's easier). Then in the innermost loop we
    // assume an encryption key and manually set the bits to values that when encrypted end up as the same
    // character as the neighbor. The encryption key assumption only has a 1/256 chance of being
    // correct so the brute force part is important to get more hits. Furthermore, the more brute force
    // we apply the better chances we have to get a high number of repeats from the checksum characters.
    
    // Since the brute force is costly we optimize by running the loop in parallel and skipping over encryption
    // keys that yield poor results. Being too aggressive with this can result in good codes being skipped.
    switch (mission_type) {
        case MISSION_TYPE_NOMRAL:
            #pragma omp parallel for
            for (int client_idx = 0; client_idx < CLIENTS_LEN; client_idx++) {
                char best_code[CODE_LEN + 1] = "";
                char current_code[CODE_LEN + 1] = "";
                char current_bitstream[BITSTREAM_FULL_LEN + 1] = "";
                int best_checksum = -1;
                strncat(current_bitstream, bitstream, BITSTREAM_FULL_LEN);
                struct evaluation best_evaluation = {0, FLT_MAX};
                NumToBits(clients[client_idx], BITS_TARGET_CLIENT, current_bitstream + POS_TARGET);
                NumToBits(clients[client_idx], BITS_TARGET_CLIENT, current_bitstream + POS_CLIENT);
                for (int flavor_text_msb = 0; flavor_text_msb < 8; flavor_text_msb++) {
                    NumToBits(flavor_text_msb, BITS_FLAVOR_TEXT_MSB, current_bitstream + POS_FLAVOR_TEXT_MSB);
                    for (int flavor_text_lsb = 0; flavor_text_lsb < 2; flavor_text_lsb++) {
                        NumToBits(flavor_text_lsb, BITS_FLAVOR_TEXT_LSB, current_bitstream + POS_FLAVOR_TEXT_LSB);
                        int checksum_repeats[256];
                        for (uint32_t checksum = 0; checksum <= 0xFF; checksum++) {
                            checksum_repeats[checksum] = INT_MAX;
                        }
                        for (int nb_sf = 0; nb_sf <= 0xFFFF; nb_sf++) {
                            NumToBits(nb_sf, BITS_NB_SF, current_bitstream);
                            for (uint32_t checksum = 0; checksum <= 0xFF; checksum++) {
                                if (checksum_repeats[checksum] < 7) {
                                    continue;
                                }
                                GenerateCode(current_bitstream, current_code, region, checksum, false);
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
                                int carry_change = 0;
                                for (int i = 0; i < 4; i++) {
                                    int character_to_change = characters_to_change[i];
                                    int neighbor_to_copy = neighbors_to_copy[i];

                                    char* bit_ptr = current_bitstream + offsets[i];
                                    char substring[5 + 1] = "";
                                    strncpy(substring, bit_ptr, 5);
                                    uint32_t num = strtol(substring, NULL, 2);

                                    uint32_t encrypted_num = reverse_lookup[(int) current_code[character_to_change]];

                                    uint32_t encryption_value = (encrypted_num - num + carry_change) % 32;

                                    uint32_t target_num = reverse_lookup[(int) current_code[neighbor_to_copy]];
                                    uint32_t target_num_decrypted;

                                    int carry_before;
                                    int carry_after;
                                    if (i == 2) {
                                        // This character is divided into two encryption bytes
                                        uint32_t target_num_msb = target_num >> 3;
                                        uint32_t target_num_lsb = target_num & 0x7;

                                        uint32_t encryption_value_msb = (((encrypted_num & 0x18) - (num & 0x18)) >> 3) % 4;
                                        uint32_t encryption_value_lsb = ((encrypted_num & 0x7) - (num & 0x7) + carry_change) % 8;

                                        uint32_t target_num_decrypted_msb = (target_num_msb - encryption_value_msb) % 4;
                                        uint32_t target_num_decrypted_lsb = (target_num_lsb - encryption_value_lsb) % 8;

                                        target_num_decrypted = (target_num_decrypted_msb << 3) | (target_num_decrypted_lsb);

                                        carry_before = (num >> 3) + encryption_value_msb >= 4;
                                        carry_after = (target_num_decrypted >> 3) + encryption_value_msb >= 4;
                                        carry_change = carry_after - carry_before;
                                    } else {
                                        target_num_decrypted = (target_num - encryption_value) % 32;
                                        // Keep track of whether there's a carry to the next character; affects encryption
                                        // Only matters when two adjacent characters share an encryption block
                                        if (i == 1) {
                                            carry_before = num + encryption_value >= 32;
                                            carry_after = target_num_decrypted + encryption_value >= 32;
                                            carry_change = carry_after - carry_before;
                                        } else {
                                            carry_change = 0;
                                        }
                                    }
                                    NumToBits(target_num_decrypted, 5, bit_ptr);
                                }

                                if (!GenerateCode(current_bitstream, current_code, region, checksum, true)) {
                                    continue;
                                }

                                struct evaluation result = EvaluateCode(current_code, best_evaluation.repeats);

                                if (result.repeats < 4) {
                                    printf("DIESOFCRINGE\n");
                                    exit(EXIT_FAILURE);
                                }

                                checksum_repeats[checksum] = result.repeats;
                                if  (result.repeats > best_evaluation.repeats || 
                                    (result.repeats == best_evaluation.repeats &&
                                        result.distance < best_evaluation.repeats))
                                {
                                    strcpy(best_code, current_code);
                                    best_evaluation = result;
                                    best_checksum = checksum;
                                }
                            }
                        }
                    }
                }
                #pragma omp atomic
                iterations += 1;

                printf("%3d(%3d) / %3d: %s (c: %2d, d: %5.1f, e: %3d)\n",
                    iterations, client_idx, CLIENTS_LEN, best_code, best_evaluation.repeats, best_evaluation.distance, best_checksum);
                strcpy(best_codes[client_idx].code, best_code);
                best_codes[client_idx].eval = best_evaluation;
            }
            break;
        default:
    }
    // Sort the results of the iterations run in parallel and return the best one
    qsort(best_codes, CLIENTS_LEN, sizeof(struct code_details), compare_codes);
    char* final_code = malloc(sizeof (char[CODE_LEN + 1]));
    strncpy(final_code, best_codes[0].code, CODE_LEN + 1);
    return final_code;
}