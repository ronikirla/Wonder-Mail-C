#include "generator.h"
#include "include.h"
#include "optimize.h"

#define POS_NULL_BITS       0
#define POS_SPECIAL_FLOOR   8
#define POS_FLAVOR_TEXT     32
#define POS_REWARD          68
#define POS_REWARD_TYPE     11
#define POS_TARGET_ITEM     83
#define POS_TARGET          104
#define POS_CLIENT          115    

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

void* SetUnks(char* code, enum region region, enum mission_type mission_type) {
    int unks[CODE_LEN];
    int unks_len = 0;

    

    for (int i = 0; i < unks_len; i++) {
        code[unks[i]] = '?';
    }
}

char* GenerateOptimizedCode(char* bitstream, enum region region, enum mission_type mission_type) {
    char* best_code = malloc(sizeof (char[CODE_LEN + 1]));
    char current_code[CODE_LEN + 1] = "";

    struct evaluation best_evaluation = {0, FLT_MAX};

    for (int i = 0; i < 0xFFFFFF; i++) {
        for (int j = 0; j < 0x1; j++) {
            // dont forget to loop through the checksums

            // Flavor text
            NumToBits(i, 24, bitstream + POS_FLAVOR_TEXT);

            // Null bits
            NumToBits(j, 8, bitstream + POS_NULL_BITS);

            GenerateCode(bitstream, current_code, region);

            struct evaluation result = EvaluateCode(current_code, best_evaluation.repeats);
            if  (result.repeats > best_evaluation.repeats || 
                (result.repeats == best_evaluation.repeats &&
                    result.distance < best_evaluation.repeats))
            {
                strcpy(best_code, current_code);
                printf("%s\n", bitstream);
                best_evaluation = result;
            }
        }
    }

    return best_code;
}