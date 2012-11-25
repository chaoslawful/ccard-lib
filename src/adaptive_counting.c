#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "murmurhash.h"
#include "loglog_counting.h"
#include "adaptive_counting.h"

/**
 * Switching empty bucket ratio
 */
static const double B_s = 0.051;

adp_cnt_ctx_t* adp_cnt_init(const void *obuf, uint32_t len_or_k)
{
    adp_cnt_ctx_t *ctx;
    ll_cnt_ctx_t *ll_ctx;
    uint8_t *buf = (uint8_t*)obuf;
    uint32_t i;

    if (len_or_k == 0) {
        // invalid buffer length or k
        return NULL;
    }

    ll_cnt_init(ll_ctx, buf, len_or_k);
    if (ll_ctx == NULL) {
        return NULL;
    }

    ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t));
    ctx->super = ll_ctx;
    ctx->b_e = 0;

    for (i = 0; i < ll_ctx->m; i++) {
        if (ll_ctx->M[i] == 0) {
            ctx->b_e++;
        }
    }

    return ctx;
}

int adp_cnt_fini(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        free(ctx->super);
        free(ctx);
        return 0;
    }

    return -1;
}

int adp_cnt_errnum(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        return ctx->super->err;
    }

    return CCARD_ERR_INVALID_CTX;
}

const char* adp_cnt_errstr(int err)
{
    static const char *msg[] = {
        "No error",
        "Invalid algorithm context",
        "Merge bitmap failed",
        NULL
    };

    if (-err >= 0 && -err < (int)(sizeof(msg)/sizeof(msg[0])-1)) {
        return msg[-err];
    }

    return "Invalid error number";
}

// vi:ft=c ts=4 sw=4 fdm=marker et

