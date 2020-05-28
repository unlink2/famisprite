#include "include/famisprite.h"

#include <stdlib.h>

#define my_malloc(x) malloc(x)

char* fami_decode(char *data, unsigned int *length, char *decoded) {
    // we need an array of lenght*4
    if (!decoded) {
        decoded = my_malloc(FAMI_BPP*2*(*length));
    }

    // 8x8 sprite
    // each tile must have at least 16 bytes
    unsigned int total_length = 0;
    for (int i = 0; i < *length; i+=FAMI_TILE_LEN*2) {
        unsigned int len = 0;
        fami_decode_tile(data+i, decoded+i*4, &len);
        total_length += len;
    }

    *length = total_length; // return total decoded length

    return decoded;
}

char *fami_decode_tile(char *data, char *decoded, unsigned int *lenght) {
    for (int i = 0; i < FAMI_TILE_LEN; i++) {
        // get first plane
        char p1 = data[i];
        // get second plane
        char p2 = data[i+FAMI_TILE_LEN];
        decoded[i*FAMI_TILE_LEN+0] = fami_decode_pixel(p1, p2, 0);
        decoded[i*FAMI_TILE_LEN+1] = fami_decode_pixel(p1, p2, 1);
        decoded[i*FAMI_TILE_LEN+2] = fami_decode_pixel(p1, p2, 2);
        decoded[i*FAMI_TILE_LEN+3] = fami_decode_pixel(p1, p2, 3);
        decoded[i*FAMI_TILE_LEN+4] = fami_decode_pixel(p1, p2, 4);
        decoded[i*FAMI_TILE_LEN+5] = fami_decode_pixel(p1, p2, 5);
        decoded[i*FAMI_TILE_LEN+6] = fami_decode_pixel(p1, p2, 6);
        decoded[i*FAMI_TILE_LEN+7] = fami_decode_pixel(p1, p2, 7);
    }
    *lenght = FAMI_TILE_LEN*FAMI_TILE_LEN;

    return decoded;
}

char *fami_encode(char *data, unsigned int *length, char *encoded) {
    // we need an array of lenght*4
    if (!encoded) {
        encoded = my_malloc((*length)/(FAMI_BPP*2));
    }

    unsigned int total_length = 0;
    for (int i = 0; i < *length; i+=FAMI_TILE_LEN*FAMI_TILE_LEN) {
        unsigned int len = 0;
        fami_encode_tile(data+i, encoded+i/4, &len);
        total_length += len;
    }
    *length = total_length;

    return encoded;
}

char* fami_encode_tile(char *data, char *encoded, unsigned int *length) {
    for (int i = 0; i < FAMI_TILE_LEN*FAMI_TILE_LEN; i += 8) {
        char p1 = 0;
        char p2 = 0;
        fami_encode_pixel(p1, p2, data[i]);
        fami_encode_pixel(p1, p2, data[i+1]);
        fami_encode_pixel(p1, p2, data[i+2]);
        fami_encode_pixel(p1, p2, data[i+3]);
        fami_encode_pixel(p1, p2, data[i+4]);
        fami_encode_pixel(p1, p2, data[i+5]);
        fami_encode_pixel(p1, p2, data[i+6]);
        fami_encode_pixel(p1, p2, data[i+7]);
        encoded[i/FAMI_TILE_LEN] = p1;
        encoded[i/FAMI_TILE_LEN+FAMI_TILE_LEN] = p2;
    }
    *length = FAMI_TILE_LEN*2;
    return encoded;
}

void fami_init_state(fami_state_t *state) {
    // empty color
    fami_color_t color = {0, 0xFF, 0x7F};
    for (int i = 0; i < FAMI_MAX_COLORS; i++) {
        state->colors[i] = color;
    }
}

void fami_set_color(fami_state_t *state, fami_color_t color, fami_color_index index) {
    index &= FAMI_MAX_COLOR_INDEX;
    state->colors[index] = color;
}

fami_color_t fami_get_color(fami_state_t *state, fami_color_index index) {
    index &= FAMI_MAX_COLOR_INDEX;
    return state->colors[index];
}
