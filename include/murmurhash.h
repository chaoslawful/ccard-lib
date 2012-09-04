#ifdef MURMURHASH_H__
#define MURMURHASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    uint32_t murmurhash(void *buf, uint32_t len, uint32_t seed);
    uint32_t murmurhash_long(uint64_t data);

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

