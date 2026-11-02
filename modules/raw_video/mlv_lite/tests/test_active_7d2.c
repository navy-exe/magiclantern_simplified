/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../active_7d2.h"

int main(int argc, char **argv)
{
    const int rows = 1088;
    size_t src_size = (size_t) rows * ACTIVE_7D2_SOURCE_PITCH;
    unsigned char *src = malloc(src_size), *original = malloc(src_size);
    unsigned char *dst = malloc(src_size + 128);
    assert(src && original && dst);
    uint32_t state = 0x7d2112;
    for (size_t i = 0; i < src_size; i++)
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        src[i] = state;
    }
    memcpy(original, src, src_size);
    int cases = 0;
    for (int height = 8; height <= 1048; height += 8)
    {
        for (int y = 0; y <= 40 && y + height <= rows; y += 2)
        {
            size_t n = (size_t) height * ACTIVE_7D2_PITCH;
            memset(dst, 0xa5, src_size + 128);
            assert(active_7d2_copy(dst + 64, n, src, src_size, rows, y, height));
            for (int row = 0; row < height; row++)
                assert(memcmp(dst + 64 + (size_t)row * ACTIVE_7D2_PITCH,
                    src + (size_t)(row + y) * ACTIVE_7D2_SOURCE_PITCH + 252,
                    ACTIVE_7D2_PITCH) == 0);
            for (int i = 0; i < 64; i++)
                assert(dst[i] == 0xa5 && dst[64+n+i] == 0xa5);
            cases++;
        }
    }
    assert(memcmp(src, original, src_size) == 0);
    memset(dst, 0xa5, src_size + 128);
    assert(!active_7d2_copy(NULL, src_size, src, src_size, rows, 0, 8));
    assert(!active_7d2_copy(dst, src_size, NULL, src_size, rows, 0, 8));
    assert(!active_7d2_copy(src, src_size, src, src_size, rows, 0, 8));
    assert(!active_7d2_copy(dst, src_size, src, src_size, rows, 1, 8));
    assert(!active_7d2_copy(dst, src_size, src, src_size, rows, -2, 8));
    assert(!active_7d2_copy(dst, src_size, src, src_size, rows, rows, 8));
    assert(!active_7d2_copy(dst, src_size, src, src_size, rows, 0, 7));
    assert(!active_7d2_copy(dst, src_size, src, src_size, rows, 0, 0));
    assert(!active_7d2_copy(dst, 1, src, src_size, rows, 0, 8));
    assert(!active_7d2_copy(dst, src_size, src, 1, rows, 0, 8));
    for (size_t i = 0; i < src_size + 128; i++) assert(dst[i] == 0xa5);
    printf("PASS: %d geometry cases, exact bytes, canaries, unchanged source, invalid inputs\n", cases);
    if (argc == 4)
    {
        int h = atoi(argv[3]);
        size_t n = (size_t)h * ACTIVE_7D2_SOURCE_PITCH;
        assert(h > 0 && h <= rows);
        FILE *f = fopen(argv[1], "rb"); assert(f);
        assert(fread(src, 1, n, f) == n); fclose(f);
        assert(active_7d2_copy(dst, src_size, src, n, h, 0, h));
        f = fopen(argv[2], "wb"); assert(f);
        assert(fwrite(dst, ACTIVE_7D2_PITCH, h, f) == (size_t)h); fclose(f);
    }
    free(src); free(original); free(dst);
    return 0;
}
