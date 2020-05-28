/**
 * Simple famicom chr-rom de/encoder
 */

#define FAMI_MAX_COLOR_INDEX 3
#define FAMI_MAX_COLORS FAMI_MAX_COLOR_INDEX+1
#define FAMI_BPP 2 // 2 bits per pixel
#define FAMI_TILE_SIZE 16 // 16 bytes
#define FAMI_TILE_LEN 8 // 8 pixels

typedef unsigned char fami_color_index;

/**
 * Simple color struct
 */
typedef struct fami_color {
    char r;
    char g;
    char b;
} fami_color_t;

/**
 * Holds color conversion state
 */
typedef struct fami_state {
    // all possible colors
    fami_color_t colors[FAMI_MAX_COLORS+1];
} fami_state_t;

/**
 * Decodes a chr-rom of a given lenght
 * Inputs:
 *  encoded chr-rom data
 *  lenght of data
 *  decoded = pre-allocated ptr to return array, if NULL it will be allocted using malloc
 * Returns:
 *  array of pixels with values from 0-3
 *  modifies lenght to equal the pixel amount
 *  NULL on error
 */
char* fami_decode(char *data, unsigned int *length, char *decoded);

/**
 * Decodes a single tile
 * Does not check data must at least be 16 bytes in size
 * decoded must be a sufficently large array for each pixel (64 bytes per tile)
 * Returns:
 *  array of pixels
 *  lenght in lenght
 *  NULL on error
 */
char *fami_decode_tile(char *data, char *decoded, unsigned int *lenght);

// decodes a single pixel at index
#define fami_decode_pixel(p1, p2, index) (((p1 >> (FAMI_TILE_LEN-1-index)) & 1) | (((p2 >> (FAMI_TILE_LEN-1-index)) & 1) << 1))

/**
 * Encodes a pixel array of lenght into
 * a chr-rom
 * Inputs:
 *  decoded chr-rom color index array
 *  lenght of array
 *  decoded = pre-allocted ptr to return array, if NULL it will be allocted using malloc
 * Returns:
 *  array of chr-rom data
 *  modifies lenght to equal the total size of the resulting chr-rom
 *  NULL on error
 */
char* fami_encode(char *data, unsigned int *length, char *encoded);

/**
 * Encodes a single tile
 * Does not chek data bounds must at least be 64 bytes
 * Returns:
 *  array of 16 2bpp char data
 *  lenght in lenght
 *  NULL on error
 */
char* fami_encode_tile(char *data, char *decoded, unsigned int *length);

// encodes a single pixel, shifts data in p1 and p2 once before encoding
#define fami_encode_pixel(p1, p2, color) p1 = (((p1 << 1)) | (color & 0x1)); p2 = (((p2 << 1)) | ((color & 0x2) >> 1))

/**
 * Sets a pixel at x/y to the given value
 * Pixel value cannot be greater than 3
 * Returns:
 *  Modifies data to the new pixel value
 */
void fami_set_pixel(char *data, unsigned int x, unsigned int y, fami_color_index index);

/**
 * Returns:
 *  pixel at x/y
 */
fami_color_index fami_get_pixel(char *data, unsigned int x, unsigned int y);

/**
 * Inits fami_state with default values
 */
void fami_init_state(fami_state_t *state);

/**
 * Sets a color to a new value at a given index
 */
void fami_set_color(fami_state_t *state, fami_color_t color, fami_color_index index);

/**
 * Returns a color value for a given index
 */
fami_color_t fami_get_color(fami_state_t *state, fami_color_index index);
