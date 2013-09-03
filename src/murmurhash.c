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
    h ^= k * m;

    k = (data >> 32) * m;
    k ^= k >> r;
    h *= m;
    h ^= k * m;

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

uint64_t murmurhash64(void *buf, uint32_t len, uint32_t seed)
{
    uint8_t *data = (uint8_t *)buf;
    uint64_t m = 0xc6a4a7935bd1e995L;
    uint32_t r = 47;
    uint64_t h = (seed & 0xffffffffl) ^ (len * m);
    uint32_t len8 = len / 8;
    uint32_t i;

    for (i = 0; i < len8; i++) {
        uint32_t i8 = i * 8;
        uint64_t k = ((uint64_t) data[i8 + 0] & 0xff) + (((uint64_t) data[i8 + 1] & 0xff) << 8)
                     + (((uint64_t) data[i8 + 2] & 0xff) << 16) + (((uint64_t) data[i8 + 3] & 0xff) << 24)
                     + (((uint64_t) data[i8 + 4] & 0xff) << 32) + (((uint64_t) data[i8 + 5] & 0xff) << 40)
                     + (((uint64_t) data[i8 + 6] & 0xff) << 48) + (((uint64_t) data[i8 + 7] & 0xff) << 56);

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    switch (len % 8) {
        case 7:
            h ^= (uint64_t) (data[(len & ~7) + 6] & 0xff) << 48;
        case 6:
            h ^= (uint64_t) (data[(len & ~7) + 5] & 0xff) << 40;
        case 5:
            h ^= (uint64_t) (data[(len & ~7) + 4] & 0xff) << 32;
        case 4:
            h ^= (uint64_t) (data[(len & ~7) + 3] & 0xff) << 24;
        case 3:
            h ^= (uint64_t) (data[(len & ~7) + 2] & 0xff) << 16;
        case 2:
            h ^= (uint64_t) (data[(len & ~7) + 1] & 0xff) << 8;
        case 1:
            h ^= (uint64_t) (data[len & ~7] & 0xff);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

uint64_t murmurhash64_no_seed(void *buf, uint32_t len)
{
    return murmurhash64(buf, len, 0xe17a1465);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

