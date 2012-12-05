#include <stdint.h>
#include "murmurhash.h"

uint32_t murmurhash(void *buf, uint32_t len, uint32_t seed)
{
    uint8_t *data = (uint8_t *)buf;
    uint32_t m = 0x5bd1e995;
    uint32_t r = 24;
    uint32_t h = seed ^ len;
    uint32_t len_4 = len >> 2;
    uint32_t i;
    uint32_t len_m;
    uint32_t left;

    for(i = 0; i < len_4; i++) {
        uint32_t i_4 = i << 2;
        uint32_t k = data[i_4 + 3];
        k <<= 8;
        k |= data[i_4 + 2] & 0xff;
        k <<= 8;
        k |= data[i_4 + 1] & 0xff;
        k <<= 8;
        k |= data[i_4 + 0] & 0xff;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
    }

    // avoid calculating modulo
    len_m = len_4 << 2;
    left = len - len_m;

    if (left != 0) {
        if (left >= 3) {
            h ^= data[len - 3] << 16;
        }
        if (left >= 2) {
            h ^= data[len - 2] << 8;
        }
        if (left >= 1) {
            h ^= data[len - 1];
        }
        h *= m;
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

uint32_t murmurhash_long(uint64_t data)
{
    uint32_t m = 0x5bd1e995;
    uint32_t r = 24;
    uint32_t h = 0;
    uint32_t k = (uint32_t)(data * m);

    k ^= k >> r;
    h ^= k*m;

    k = (data >> 32) * m;
    k ^= k >> r;
    h *= m;
    h ^= k*m;

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

// vi:ft=c ts=4 sw=4 fdm=marker et

