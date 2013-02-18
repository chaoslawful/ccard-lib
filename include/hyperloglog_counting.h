#ifndef HYPERLOGLOG_COUNTING_H__
#define HYPERLOGLOG_COUNTING_H__

#include "ccard_common.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Opaque hyperloglog counting context type
     * */
    typedef struct hll_cnt_ctx_s hll_cnt_ctx_t;

    /**
     * Initialize hyperloglog counting context with optional raw bitmap.
     *
     * @param[in] buf Pointer to the raw bitmap. NULL if there's none.
     * @param[in] len_or_k The length of the bitmap if buf is not NULL;
     * otherwise it's the base-2 logarithm of the bitmap length.
     * @param[in] hf Hash function that be applied to elements.
     *
     * @retval not-NULL An initialized context to be used with the rest of
     * methods.
     * @retval NULL If error occured.
     *
     * @see hll_cnt_fini, hll_cnt_init
     * */
    hll_cnt_ctx_t  *hll_cnt_raw_init(const void *obuf, uint32_t len_or_k,
                                     uint8_t hf);

    /**
     * Initialize hyperloglog counting context with optional serialized bitmap.
     *
     * @param[in] buf Pointer to the serialized bitmap. NULL if there's none.
     * @param[in] len_or_k The length of the bitmap if buf is not NULL;
     * otherwise it's the base-2 logarithm of the bitmap length.
     * @param[in] hf Hash function that be applied to elements.
     *
     * @retval not-NULL An initialized context to be used with the rest of
     * methods.
     * @retval NULL If error occured.
     *
     * @see hll_cnt_fini, hll_cnt_raw_init
     * */
    hll_cnt_ctx_t  *hll_cnt_init(const void *obuf, uint32_t len_or_k,
                                 uint8_t hf);

    /**
     * Retrieve the cardinality calculated from bitmap in the context using
     * Hyperloglog Counting.
     *
     * @param[in] ctx Pointer to the context.
     *
     * @retval >=0 Calculated cardinality based on bitmap in the context if
     * success.
     * @retval -1 If error occured.
     *
     * @see hll_cnt_offer, hll_cnt_reset
     * */
    int64_t         hll_cnt_card(hll_cnt_ctx_t *ctx);

    /**
     * Offer a object to be distinct counted.
     *
     * @param[in,out] ctx Pointer to the context.
     * @param[in] buf Pointer to the buffer storing object.
     * @param[in] len The length of the buffer.
     *
     * @retval 1 If the object affected final counting.
     * @retval 0 If final counting isn't affected by the object.
     * @retval -1 If error occured.
     *
     * @see hll_cnt_card, hll_cnt_reset
     * */
    int             hll_cnt_offer(hll_cnt_ctx_t *ctx, const void *buf,
                                  uint32_t len);

    /**
     * Reset bitmap in the context, effectively clear cardinality to zero.
     *
     * @param[in,out] ctx Pointer to the context.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     *
     * @see hll_cnt_card, hll_cnt_offer
     * */
    int             hll_cnt_reset(hll_cnt_ctx_t *ctx);

    /**
     * Get the raw bitmap or bitmap length from context.
     *
     * @param[in] ctx Pointer to the context.
     * @param[out] buf Pointer to buffer storing returning bitmap. NULL if only
     * bitmap length is needed.
     * @param[out] len Pointer to variable storing returning bitmap length.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     *
     * @see hll_cnt_merge, hll_cnt_merge_raw_bytes, hll_cnt_get_bytes
     * */
    int             hll_cnt_get_raw_bytes(hll_cnt_ctx_t *ctx, void *buf,
                                          uint32_t *len);

    /**
     * Get the serialized bitmap or bitmap length from context.
     *
     * @param[in] ctx Pointer to the context.
     * @param[out] buf Pointer to buffer storing returning bitmap. NULL if only
     * bitmap length is needed.
     * @param[out] len Pointer to variable storing returning bitmap length.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     *
     * @see hll_cnt_merge, hll_cnt_merge_bytes, hll_cnt_get_raw_bytes
     * */
    int             hll_cnt_get_bytes(hll_cnt_ctx_t *ctx, void *buf,
                                      uint32_t *len);

    /**
     * Merge several hyperloglog counting context into the current one,
     * effectively combined all distinct countings.
     *
     * Usage:
     * @code{c}
     * if(hll_cnt_merge(ctx, ctx_1, ctx_2, ..., ctx_n, NULL)) {
     *     printf("Failed to merge contexts: %s",
     *            hll_cnt_errstr(hll_cnt_errnum(ctx)));
     * }
     * @endcode
     *
     * @note All context to be merged must be of the same bitmap length,
     * otherwise error will be returned!
     *
     * @param[in,out] ctx Pointer to the context merging to.
     * @param[in] tbm Pointer to the first context to be merged. The rest
     * contexts will be listed sequentially with a ending NULL.
     *
     * @retval 0 if all were merged successfully.
     * @retval -1 if error occured.
     *
     * @see hll_cnt_merge_bytes, hll_cnt_get_bytes
     * */
    int             hll_cnt_merge(hll_cnt_ctx_t *ctx, hll_cnt_ctx_t *tbm,
                                  ...);

    /**
     * Merge several hyperloglog counting bitmap into the current context,
     * effectively combined all distinct countings.
     *
     * Usage:
     * @code{c}
     * if(hll_cnt_merge_raw_bytes(ctx, buf_1, len_1, buf_2, len_2,
     *     ..., buf_n, len_n, NULL)) {
     *     printf("Failed to merge bitmaps: %s",
     *            hll_cnt_errstr(hll_cnt_errnum(ctx)));
     * }
     * @endcode
     *
     * @note All bitmap to be merged must be of the same length with the bitmap
     * in current context, otherwise error will be returned!
     *
     * @param[in,out] ctx Pointer to the context merging to.
     * @param[in] buf Pointer to the first bitmap to be merged.
     * @param[in] len Length of the first bitmap to be merged. The rest buf/len
     * pairs will be listed sequentially with a ending NULL.
     *
     * @retval 0 if all were merged successfully.
     * @retval -1 if error occured.
     *
     * @see hll_cnt_merge, hll_cnt_get_bytes, hll_cnt_merge_bytes
     * */
    int             hll_cnt_merge_raw_bytes(hll_cnt_ctx_t *ctx,
                                            const void *buf, uint32_t len,
                                            ...);

    /**
     * Merge several hyperloglog counting bitmap into the current context,
     * effectively combined all distinct countings.
     *
     * Usage:
     * @code{c}
     * if(hll_cnt_merge_bytes(ctx, buf_1, len_1, buf_2, len_2,
     *     ..., buf_n, len_n, NULL)) {
     *     printf("Failed to merge bitmaps: %s",
     *            hll_cnt_errstr(hll_cnt_errnum(ctx)));
     * }
     * @endcode
     *
     * @note All bitmap to be merged must be of the same length with the bitmap
     * in current context, otherwise error will be returned!
     *
     * @param[in,out] ctx Pointer to the context merging to.
     * @param[in] buf Pointer to the first bitmap to be merged.
     * @param[in] len Length of the first bitmap to be merged. The rest buf/len
     * pairs will be listed sequentially with a ending NULL.
     *
     * @retval 0 if all were merged successfully.
     * @retval -1 if error occured.
     *
     * @see hll_cnt_merge, hll_cnt_get_bytes, hll_cnt_merge_raw_bytes
     * */
    int             hll_cnt_merge_bytes(hll_cnt_ctx_t *ctx,
                                        const void *buf, uint32_t len,
                                        ...);

    /**
     * Finalize and release resources of the given hyperloglog counting
     * context.
     *
     * @param[in] ctx Pointer to the context to release.
     *
     * @retval 0 if finalized successfully.
     * @retval -1 if error occured.
     *
     * @see hll_cnt_init
     * */
    int             hll_cnt_fini(hll_cnt_ctx_t *ctx);

    /**
     * Get error status of the given context.
     *
     * @param[in] ctx Pointer to the context.
     *
     * @retval <=0 Error number in the context.
     *
     * @see hll_cnt_errstr
     * */
    int             hll_cnt_errnum(hll_cnt_ctx_t *ctx);

    /**
     * Convert error status to human-friendly message.
     *
     * @param[in] errn Error number returned by hll_cnt_errnum.
     *
     * @retval not-NULL Corresponding message string.
     * @retval NULL Invalid error number.
     *
     * @see hll_cnt_errnum
     * */
    const char     *hll_cnt_errstr(int errn);

    /**
     * Hyperloglog counting algorithm definition
     * */
    extern ccard_algo_t *hll_algo;

#ifdef __cplusplus
}
#endif

#endif

/* vi:ft=c ts=4 sw=4 fdm=marker et
 * */

