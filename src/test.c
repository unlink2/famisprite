#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "include/famisprite.h"
#include "include/utility.h"

char assert_color_equal(fami_color_t c1, fami_color_t c2) {
    return ((c1.r & 0xFF) == (c2.r & 0xFF)) &&
        ((c1.g & 0xFF) == (c2.g & 0xFF)) &&
        ((c1.b & 0xFF) == (c2.b & 0xFF));
}

static void test_fami_state_init(void **state) {
    fami_state_t fstate;
    fami_init_state(&fstate);

    fami_color_t d = {0x00, 0xFF, 0x7F};

    for (int i = 0; i < FAMI_MAX_COLORS; i++) {
        assert_true(assert_color_equal(fstate.colors[i], d));
    }
}

static void test_fami_set_color(void **state) {
    fami_state_t fstate;
    fami_init_state(&fstate);
    fami_color_t c1 = {0x12, 0xAB, 0xC1};

    fami_set_color(&fstate, c1, 1);
    assert_false(assert_color_equal(fstate.colors[0], c1));
    assert_false(assert_color_equal(fstate.colors[2], c1));
    assert_false(assert_color_equal(fstate.colors[3], c1));
    assert_true(assert_color_equal(fstate.colors[1], c1));

    // should wrap
    fami_set_color(&fstate, c1, 4); // index 0!
    assert_true(assert_color_equal(fami_get_color(&fstate, 0), c1));
    assert_false(assert_color_equal(fami_get_color(&fstate, 2), c1));
    assert_false(assert_color_equal(fami_get_color(&fstate, 3), c1));
    assert_true(assert_color_equal(fami_get_color(&fstate, 1), c1));
}

// test sprite from nesdev wiki
const char test_sprite[16*3] = {
    0x41, 0xC2, 0x44, 0x48, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x16, 0x21, 0x42, 0x87,

    0x41, 0xC2, 0x44, 0x48, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x16, 0x21, 0x42, 0x87,

    0x41, 0xC2, 0x44, 0x48, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x16, 0x21, 0x42, 0x87
};

const char test_sprite_decoded[64*3] = {
    0, 1, 0, 0, 0, 0, 0, 3,
    1, 1, 0, 0, 0, 0, 3, 0,
    0, 1, 0, 0, 0, 3, 0, 0,
    0, 1, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 2, 2, 0,
    0, 0, 3, 0, 0, 0, 0, 2,
    0, 3, 0, 0, 0, 0, 2, 0,
    3, 0, 0, 0, 0, 2, 2, 2,

    0, 1, 0, 0, 0, 0, 0, 3,
    1, 1, 0, 0, 0, 0, 3, 0,
    0, 1, 0, 0, 0, 3, 0, 0,
    0, 1, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 2, 2, 0,
    0, 0, 3, 0, 0, 0, 0, 2,
    0, 3, 0, 0, 0, 0, 2, 0,
    3, 0, 0, 0, 0, 2, 2, 2,

    0, 1, 0, 0, 0, 0, 0, 3,
    1, 1, 0, 0, 0, 0, 3, 0,
    0, 1, 0, 0, 0, 3, 0, 0,
    0, 1, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 2, 2, 0,
    0, 0, 3, 0, 0, 0, 0, 2,
    0, 3, 0, 0, 0, 0, 2, 0,
    3, 0, 0, 0, 0, 2, 2, 2
};

static void test_fami_decode_pixel(void **state) {
    for (int i = 0; i < FAMI_TILE_LEN; i++) {
        char p1 = test_sprite[i];
        char p2 = test_sprite[FAMI_TILE_LEN+i];
        assert_int_equal(fami_decode_pixel(p1, p2, 0), test_sprite_decoded[0+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 1), test_sprite_decoded[1+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 2), test_sprite_decoded[2+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 3), test_sprite_decoded[3+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 4), test_sprite_decoded[4+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 5), test_sprite_decoded[5+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 6), test_sprite_decoded[6+i*8]);
        assert_int_equal(fami_decode_pixel(p1, p2, 7), test_sprite_decoded[7+i*8]);
    }
}

static void test_fami_decode_tile(void **state) {
    char decoded[64];
    unsigned int len = 0;
    fami_decode_tile((char*)test_sprite, (char*)decoded, &len);
    assert_int_equal(len, 64);
    for (int i = 0; i < 64; i++) {
        assert_int_equal(test_sprite_decoded[i], decoded[i]);
    }
}

static void test_fami_decode(void **state) {
    char decoded[64*3];
    unsigned int len = 16*3; // 16 bytes
    assert_ptr_equal(fami_decode((char*)test_sprite, &len, (char*)decoded), (char*)decoded);
    assert_int_equal(len, 64*3);
    for (int i = 0; i < len; i++) {
        assert_int_equal(test_sprite_decoded[i], decoded[i]);
    }
}

static void test_fami_encode_pixel(void **state) {
    for (int i = 0; i < FAMI_TILE_LEN*FAMI_TILE_LEN; i+=FAMI_TILE_LEN) {
        char p1 = 0;
        char p2 = 0;
        fami_encode_pixel(p1, p2, test_sprite_decoded[i]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+1]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+2]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+3]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+4]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+5]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+6]);
        fami_encode_pixel(p1, p2, test_sprite_decoded[i+7]);
        assert_int_equal(test_sprite[i/FAMI_TILE_LEN], p1);
        assert_int_equal(test_sprite[i/FAMI_TILE_LEN+FAMI_TILE_LEN], p2);
    }
}

static void test_fami_encode_tile(void **state) {
    char encoded[16];
    unsigned int len = 0;
    fami_encode_tile((char*)test_sprite_decoded, (char*)encoded, &len);
    assert_int_equal(len, 16);
    for (int i = 0; i < FAMI_TILE_LEN*2; i++) {
        assert_int_equal(test_sprite[i], encoded[i]);
    }
}

static void test_fami_encode(void **state) {
    char encoded[16*3];
    unsigned int len = 64*3;
    assert_ptr_equal(fami_encode((char*)test_sprite_decoded, &len, (char*)encoded), (char*)encoded);
    assert_int_equal(len, 16*3);

    for (int i = 0; i < len; i++) {
        assert_int_equal(test_sprite[i], encoded[i]);
    }
}

static void test_fami_set_pixel(void **state) {
    char decoded[128];
    unsigned int len = 32;
    assert_ptr_equal(fami_decode((char*)test_sprite, &len, (char*)decoded), (char*)decoded);

    // set sprite color
    assert_int_equal(fami_get_pixel(decoded, 5, 10), 3);
    fami_set_pixel(decoded, 5, 10, 1);
    assert_int_equal(fami_get_pixel(decoded, 5, 10), 1);
}

static void test_parse_arg(void **state) {
    arg a1 = parse_arg("testargument", "test");

    assert_string_equal(a1.key, "test");
    assert_string_equal(a1.value, "argument");

    arg a2 = parse_arg("--flag", "--flag");

    assert_string_equal(a2.key, "--flag");
    assert_int_equal(a2.value[0], '\0');

    arg a3 = parse_arg("--flag", "--o");

    assert_null(a3.key);
    assert_null(a3.value);

    arg a4 = parse_arg("-otest", "-o");
    assert_string_equal(a4.key, "-o");
    assert_string_equal(a4.value, "test");
}

static void test_is_arg(void **state) {
    assert_true(is_arg("-htest", "-h"));
    assert_true(is_arg("--h", "--h"));
    assert_false(is_arg("-h", "--h"));
    assert_false(is_arg("--otest", "-h"));
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fami_state_init),
        cmocka_unit_test(test_fami_set_color),
        cmocka_unit_test(test_fami_decode_pixel),
        cmocka_unit_test(test_fami_decode_tile),
        cmocka_unit_test(test_fami_decode),
        cmocka_unit_test(test_fami_encode_pixel),
        cmocka_unit_test(test_fami_encode_tile),
        cmocka_unit_test(test_fami_encode),
        cmocka_unit_test(test_fami_set_pixel),
        cmocka_unit_test(test_parse_arg),
        cmocka_unit_test(test_is_arg)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
