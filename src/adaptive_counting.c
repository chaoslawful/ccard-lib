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

static uint8_t num_of_trail_zeros(uint32_t i)
{
    uint32_t y;
    uint8_t n = 31;

    if (i == 0)
        return 32;

    y = i << 16;    if (y != 0) { n -= 16; i = y; }
    y = i << 8;     if (y != 0) { n -= 8; i = y; }
    y = i << 4;     if (y != 0) { n -= 4; i = y; }
    y = i << 2;     if (y != 0) { n -= 2; i = y; }
    return n - (uint8_t)((i << 1) >> 31);
}

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

    ll_ctx = ll_cnt_init(buf, len_or_k);
    if (ll_ctx == NULL) {
        return NULL;
    }

    ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t));
    ctx->super = ll_ctx;
    ctx->b_e = ll_ctx->m;

    if (obuf) {
        for (i = 0; i < ll_ctx->m; i++) {
            if (ll_ctx->M[i] != 0) {
                ctx->b_e--;
            }
        }
    }

    return ctx;
}

int64_t adp_cnt_card(adp_cnt_ctx_t *ctx)
{
    double B = ctx->b_e / (double)ctx->super->m;

    if (!ctx || !ctx->super) {
        return -1;
    }
    ctx->super->err = CCARD_OK;

    if (B >= B_s) {
        return (int64_t)round((-(double)ctx->super->m) * log(B));
    }

    return ll_cnt_card(ctx->super);
}

int adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len)
{
    int modified = 0;
    uint32_t x, j;
    uint8_t r;

    if (!ctx || !ctx->super) {
        return -1;
    }

    ctx->super->err = CCARD_OK;
    /* TODO: use lookup3 hash */
    x = murmurhash((void *)buf, len, -1);
    j = x >> (32 - ctx->super->k);
    r = (uint8_t)(num_of_trail_zeros(x << ctx->super->k) - ctx->super->k + 1);
    if (ctx->super->M[j] < r) {
        ctx->super->Rsum += r - ctx->super->M[j];
        if (ctx->super->M[j] == 0) {
            ctx->b_e--;
        }
        ctx->super->M[j] = r;

        modified = 1;
    }

    return modified;
}

int adp_cnt_reset(adp_cnt_ctx_t *ctx)
{
    if (!ctx || !ctx->super) {
        return -1;
    }

    ctx->super->err = CCARD_OK;
    ll_cnt_reset(ctx->super);
    ctx->b_e = ctx->super->m;

    return 0;
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

