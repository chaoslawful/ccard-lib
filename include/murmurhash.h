#ifdef MURMURHASH_H__
#define MURMURHASH_H__

#ifdef __cplusplus
extern "C" {
#endif

	uint32_t murmurhash(void *buf, uint32_t len, uint32_t seed);
	uint32_t murmurhash_long(uint64_t data);

#ifdef __cplusplus
}
#endif

#endif

