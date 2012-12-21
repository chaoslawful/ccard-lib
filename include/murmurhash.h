#ifndef MURMURHASH_H__
#define MURMURHASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern          "C" {
#endif

    /**
     * Generate 32bit hash code of the given data using Murmurhash algorithm.
     *
     * @param buf Pointer to the data buffer
     * @param len Data length
     * @param seed Initial hash seed to saltify result
     *
     * @return Calculated hash code.
     * */
    uint32_t        murmurhash(void *buf, uint32_t len, uint32_t seed);

    /**
     * Generate 32bit hash code of the given 64bit integer using Murmurhash
     * algorithm.
     *
     * @param data The 64bit integer to be hashed
     *
     * @return Calculated hash code.
     * */
    uint32_t        murmurhash_long(uint64_t data);

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

