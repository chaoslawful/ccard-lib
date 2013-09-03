#ifndef SPARSE_BITMAP_H__
#define SPARSE_BITMAP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HF(opt) ((opt) & 0x0f)
#define IS_SPARSE_BMP(bmp) (bmp[0] & 0x80)
#define MAKE_SPARSE_ID(k) (0x80 | (k))
#define K_FROM_ID(id) (0x7f & (id))

/**
 * Additional options
 * */
enum {
    CCARD_OPT_SPARSE = 0x10
};

/**
 * Convert little-endian byte-seq starting at specified position to
 * integer.
 *
 * @param[in] bmp Pointer to the original byte array
 * @param[in] off Byte-sequence starting offset in bytes
 * @param[in] len Byte-sequence length in bytes
 * @retval Converted integer.
 * */
int sparse_bytes_to_int(const uint8_t *bmp, int off, int len);

/**
 * Convert the given integer to little-endian byte-seq starting at
 * specified position.
 *
 * @param[in] bmp Pointer to the resulting byte array
 * @param[in] off Resulting byte-sequence starting offset in bytes
 * @param[in] len Byte-sequence length in bytes
 * @param[in] val Integer to be converted
 * */
void sparse_int_to_bytes(uint8_t *bmp, int off, int len, int val);

#ifdef __cplusplus
}
#endif

#endif
/* vi:ft=c ts=4 sw=4 fdm=marker et
 * */

