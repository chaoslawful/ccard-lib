#ifndef ADAPTIVE_COUNTING_H__
#define ADAPTIVE_COUNTING_H__

#include "ccard_common.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Opaque adaptive counting context type
     * */
    typedef struct adp_cnt_ctx_s adp_cnt_ctx_t;

    /**
     * Initialize adaptive counting context with optional serialized bitmap.
     *
     * @param[in] buf Pointer to the serialized bitmap (with 3 bytes header). NULL if there's none.
     * @param[in] len_or_k The length of the bitmap if buf is not NULL;
     * otherwise it's the base-2 logarithm of the bitmap length (exclude 3 bytes header).
     * @param[in] hf Hash function that be applied to elements.
     *
     * @retval not-NULL An initialized context to be used with the rest of
     * methods.
     * @retval NULL If error occured.
     *
     * @see adp_cnt_fini
     * */
    adp_cnt_ctx_t* adp_cnt_init(const void *obuf, uint32_t len_or_k, uint8_t hf);

    /**
     * Retrieve the cardinality calculated from bitmap in the context using LogLog Counting.
     *
     * @param[in] ctx Pointer to the context.
     *
     * @retval >=0 Calculated cardinality based on bitmap in the context if
     * success.
     * @retval -1 If error occured.
     *
     * @see adp_cnt_offer, adp_cnt_reset
     * */
    int64_t adp_cnt_card_loglog(adp_cnt_ctx_t *ctx);

    /**
     * Retrieve the cardinality calculated from bitmap in the context using Adaptive Counting.
     *
     * @param[in] ctx Pointer to the context.
     *
     * @retval >=0 Calculated cardinality based on bitmap in the context if
     * success.
     * @retval -1 If error occured.
     *
     * @see adp_cnt_offer, adp_cnt_reset
     * */
    int64_t adp_cnt_card(adp_cnt_ctx_t *ctx);

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
     * @see adp_cnt_card, adp_cnt_reset
     * */
    int adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len);

    /**
     * Reset bitmap in the context, effectively clear cardinality to zero.
     *
     * @param[in,out] ctx Pointer to the context.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     *
     * @see adp_cnt_card, adp_cnt_offer
     * */
    int adp_cnt_reset(adp_cnt_ctx_t *ctx);

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
     * @see adp_cnt_merge, adp_cnt_merge_bytes
     * */
    int adp_cnt_get_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len);

    /**
     * Merge several adaptive counting context into the current one,
     * effectively combined all distinct countings.
     *
     * Usage:
     * @code{c}
     * if(adp_cnt_merge(ctx, ctx_1, ctx_2, ..., ctx_n, NULL)) {
     *     printf("Failed to merge contexts: %s",
     *            adp_cnt_errstr(adp_cnt_errnum(ctx)));
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
     * @see adp_cnt_merge_bytes, adp_cnt_get_bytes
     * */
    int adp_cnt_merge(adp_cnt_ctx_t *ctx, adp_cnt_ctx_t *tbm, ...);

    /**
     * Merge several adaptive counting bitmap into the current context,
     * effectively combined all distinct countings.
     *
     * Usage:
     * @code{c}
     * if(adp_cnt_merge_bytes(ctx, buf_1, len_1, buf_2, len_2,
     *     ..., buf_n, len_n, NULL)) {
     *     printf("Failed to merge bitmaps: %s",
     *            adp_cnt_errstr(adp_cnt_errnum(ctx)));
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
     * @see adp_cnt_merge, adp_cnt_get_bytes
     * */
    int adp_cnt_merge_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len,
            ...);

    /**
     * Finalize and release resources of the given adaptive counting context.
     *
     * @param[in] ctx Pointer to the context to release.
     * 
     * @retval 0 if finalized successfully.
     * @retval -1 if error occured.
     *
     * @see adp_cnt_init
     * */
    int adp_cnt_fini(adp_cnt_ctx_t *ctx);

    /**
     * Get error status of the given context.
     *
     * @param[in] ctx Pointer to the context.
     *
     * @retval <=0 Error number in the context.
     *
     * @see adp_cnt_errstr
     * */
    int adp_cnt_errnum(adp_cnt_ctx_t *ctx);

    /**
     * Convert error status to human-friendly message.
     *
     * @param[in] errn Error number returned by adp_cnt_errnum.
     * 
     * @retval not-NULL Corresponding message string.
     * @retval NULL Invalid error number.
     *
     * @see adp_cnt_errnum
     * */
    const char* adp_cnt_errstr(int errn);

    /**
     * Adaptive counting algorithm definition
     * */
    extern ccard_algo_t *adp_algo;

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

