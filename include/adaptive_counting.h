#ifndef ADAPTIVE_COUNTING_H__
#define ADAPTIVE_COUNTING_H__

#include <stdint.h>
#include "ccard_common.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct adp_cnt_ctx adp_cnt_ctx_t;

    /**
     * Initialize adaptive counting context with optional serialized bitmap.
     *
     * @param buf Pointer to the serialized bitmap. NULL if there's none.
     * @param len The length of the bitmap if @buf is not NULL; otherwise it's
     * the base-2 logarithm of the bitmap length.
     *
     * @retval An initialized context to be used with the rest of methods, NULL
     * if error occured.
     * */
    adp_cnt_ctx_t* adp_cnt_init(const void *buf, uint32_t len_or_k);

    /**
     * Retrieve the cardinality calculated from bitmap in the context.
     *
     * @param ctx Pointer to the context.
     *
     * @retval Calculated cardinality based on bitmap in the context if
     * success, otherwise -1.
     * */
    int64_t adp_cnt_card(adp_cnt_ctx_t *ctx);

    /**
     * Offer a object to be distinct counted.
     *
     * @param ctx Pointer to the context.
     * @param buf Pointer to the buffer storing object.
     * @param len The length of the buffer.
     *
     * @retval 1 if the object affected final counting, otherwise 0; -1 if
     * error occured.
     * */
    int adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len);

    /**
     * Get the serialized bitmap or bitmap length from context.
     *
     * @param ctx Pointer to the context.
     * @param buf Pointer to buffer storing returning bitmap. NULL if only
     * bitmap length is needed.
     * @param len Pointer to variable storing returning bitmap length.
     *
     * @retval 0 if success, otherwise -1.
     * */
    int adp_cnt_get_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len);

    /**
     * Merge several adaptive counting context into the current one,
     * effectively combined all distinct countings.
     *
     * @param ctx Pointer to the context merging to.
     * @param tbm Pointer to the first context merging from. The rest contexts
     * will be listed ending with NULL.
     *
     * @retval 0 if all were merged successfully, otherwise -1.
     * */
    int adp_cnt_merge(adp_cnt_ctx_t *ctx, adp_cnt_ctx_t *tbm, ...);

    /**
     * Get error status of the given context.
     *
     * @param ctx Pointer to the context.
     *
     * @retval Error number in the context.
     *
     * @seealso adp_cnt_errstr
     * */
    int adp_cnt_errnum(adp_cnt_ctx_t *ctx);

    /**
     * Convert error status to human-friendly message.
     *
     * @param errn Error number returned by adp_cnt_errnum.
     * 
     * @retval Corresponding message string.
     *
     * @seealso adp_cnt_errnum
     * */
    const char* adp_cnt_errstr(int errn);

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

