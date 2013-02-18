#include "sparse_bitmap.h"

int sparse_bytes_to_int(const uint8_t *bmp, int off, int len)
{
    int r = 0;
    int i;

    for(i = len - 1; i >= 0; i--) {
        r = (r << 8) | bmp[off + i];
    }

    return r;
}


void sparse_int_to_bytes(uint8_t *bmp, int off, int len, int val)
{
    int i;
    int tmp = val;

    for(i = 0; i < len; i++) {
        bmp[off + i] = tmp & 0xff;
        tmp >>= 8;
    }
}

