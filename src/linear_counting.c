#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "murmurhash.h"
#include "lookup3hash.h"
#include "linear_counting.h"

struct lnr_cnt_ctx_s {
    int err;
    uint32_t m;
    uint32_t length;
    uint32_t count;
    uint8_t hf;
    uint8_t M[1];
};

static uint8_t count_ones(uint8_t b)
{
    uint8_t ones = 0;

    while (b > 0) {
        ones += (b & 0x01);
        b >>= 1;
    }

    return ones;
}

static uint8_t calc_log2m(uint32_t m)
{
    uint8_t log2m = 1;

    while((m & 0x01) == 0) {
        log2m++;
        m >>= 1;
    }

    return log2m;
}

lnr_cnt_ctx_t *lnr_cnt_raw_init(const void *obuf, uint32_t len_or_k, uint8_t hf)
{
    lnr_cnt_ctx_t *ctx;
    uint8_t *buf = (uint8_t *)obuf;
    uint32_t i;

    if (len_or_k == 0) {
        // invalid buffer length or k
        return NULL;
    }

    if (buf) {
        // initial bitmap was given
        if ((len_or_k & (len_or_k - 1)) != 0) {
            // invalid buffer size, its length must be a power of 2
            return NULL;
        }

        ctx = (lnr_cnt_ctx_t *)malloc(sizeof(lnr_cnt_ctx_t) + len_or_k - 1);
        ctx->m = len_or_k;
        ctx->length = 8 * ctx->m;
        ctx->count = ctx->length;
        memcpy(ctx->M, buf, len_or_k);

        for (i = 0; i < len_or_k; i++) {
            ctx->count -= count_ones(ctx->M[i]);
        }
    } else {
        // k was given
        ctx = (lnr_cnt_ctx_t *)malloc(sizeof(lnr_cnt_ctx_t) + (1 << len_or_k) - 1);
        ctx->m = (1 << len_or_k);
        ctx->length = 8 * ctx->m;
        ctx->count = ctx->length;
        memset(ctx->M, 0, ctx->m);
    }
    ctx->err = CCARD_OK;
    ctx->hf = hf;

    return ctx;
}

lnr_cnt_ctx_t *lnr_cnt_init(const void *obuf, uint32_t len_or_k, uint8_t hf)
{
    uint8_t *buf = (uint8_t *)obuf;

    if (buf) {
        // initial bitmap was given
        if(len_or_k <= 3) {
            return NULL;
        }

        uint32_t data_segment_size = len_or_k - 3;
        uint8_t log2m = calc_log2m(data_segment_size);

        if (buf[0] != CCARD_ALGO_LINEAR ||
            buf[1] != hf ||
            buf[2] != log2m) {

            // counting algorithm, hash function or length not match
            return NULL;
        }

        return lnr_cnt_raw_init(buf + 3, data_segment_size, hf);
    }

    return lnr_cnt_raw_init(NULL, len_or_k, hf);
}

int64_t lnr_cnt_card(lnr_cnt_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    return (int64_t)round(ctx->length * (log(ctx->length / (double)ctx->count)));
}

int lnr_cnt_offer(lnr_cnt_ctx_t *ctx, const void *buf, uint32_t len)
{
    int modified = 0;
    uint64_t hash;
    uint32_t bit, i;
    uint8_t b, mask;

    if (!ctx) {
        return -1;
    }

    switch (ctx->hf) {
        case CCARD_HASH_LOOKUP3:
            hash = lookup3ycs64_2((const char *)buf);
            break;
        case CCARD_HASH_MURMUR:
        default:
            /* default to use murmurhash function */
            hash = (uint64_t)murmurhash((void *)buf, len, -1);
    }

    bit = (uint32_t)((hash & 0xFFFFFFFF) % (uint64_t)ctx->length);
    i = bit / 8;
    b = ctx->M[i];
    mask = (uint8_t)(1 << (bit % 8));
    if ((mask & b) == 0) {
        ctx->M[i] = (uint8_t)(b | mask);
        ctx->count--;
        modified = 1;
    }

    ctx->err = CCARD_OK;
    return modified;
}

int lnr_cnt_get_raw_bytes(lnr_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || !len || (buf && *len < ctx->m + 3)) {
        return -1;
    }

    if (buf) {
        memcpy(out, ctx->M, ctx->m);
    }
    *len = ctx->m;

    return 0;
}

int lnr_cnt_get_bytes(lnr_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    /*
     +--------------+---------+------------------------------+-----------+
     | algorithm[1] | hash[1] | bitmap length(base-2 log)[1] | bitmap[n] |
     +--------------+---------+------------------------------+-----------+
     */
    uint8_t algo = CCARD_ALGO_LINEAR;
    uint8_t *out = (uint8_t *)buf;
    uint8_t log2m = calc_log2m(ctx->m);

    if (!ctx || !len || (buf && *len < ctx->m + 3)) {
        return -1;
    }

    if (buf) {
        out[0] = algo;
        out[1] = ctx->hf;
        out[2] = log2m;
        memcpy(&out[3], ctx->M, ctx->m);
    }
    *len = ctx->m + 3;

    return 0;
}

int lnr_cnt_merge(lnr_cnt_ctx_t *ctx, lnr_cnt_ctx_t *tbm, ...)
{
    va_list vl;
    lnr_cnt_ctx_t *bm;
    uint32_t i;

    if (!ctx) {
        return -1;
    }

    if (tbm) {
        /* Cannot merge bitmap of different sizes or different hash functions */
        if ((tbm->m != ctx->m) || (tbm->hf != ctx->hf)) {
            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        for (i = 0; i < ctx->m; i++) {
            ctx->M[i] |= tbm->M[i];
        }

        va_start(vl, tbm);
        while ((bm = va_arg(vl, lnr_cnt_ctx_t *)) != NULL) {
            if ((tbm->m != ctx->m) || (tbm->hf != ctx->hf)) {
                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            for (i = 1; i < ctx->m; i++) {
                ctx->M[i] |= tbm->M[i];
            }
        }
        va_end(vl);

        ctx->count = ctx->length;
        for (i = 0; i < ctx->m; i++) {
            ctx->count -= count_ones(ctx->M[i]);
        }
    }

    ctx->err = CCARD_OK;
    return 0;
}

int lnr_cnt_merge_raw_bytes(lnr_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    va_list vl;
    uint8_t *in;
    lnr_cnt_ctx_t *bctx;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        in = (uint8_t *)buf;
        /* Cannot merge bitmap of different sizes */
        if (ctx->m != len) {
            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        bctx = lnr_cnt_raw_init(in, ctx->m, ctx->hf);
        lnr_cnt_merge(ctx, bctx, NULL);
        lnr_cnt_fini(bctx);

        va_start(vl, len);
        while ((in = (uint8_t *)va_arg(vl, const void *)) != NULL) {
            len = va_arg(vl, uint32_t);

            if (ctx->m != len) {
                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            bctx = lnr_cnt_raw_init(in, ctx->m, ctx->hf);
            lnr_cnt_merge(ctx, bctx, NULL);
            lnr_cnt_fini(bctx);
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int lnr_cnt_merge_bytes(lnr_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    va_list vl;
    uint8_t *in;
    lnr_cnt_ctx_t *bctx;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        in = (uint8_t *)buf;
        /* Cannot merge bitmap of different sizes,
        different hash functions or different algorithms */
        if ((ctx->m + 3 != len) ||
            (in[0] != CCARD_ALGO_LINEAR) ||
            (in[1] != ctx->hf)) {

            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        bctx = lnr_cnt_init(in, ctx->m, ctx->hf);
        lnr_cnt_merge(ctx, bctx, NULL);
        lnr_cnt_fini(bctx);

        va_start(vl, len);
        while ((in = (uint8_t *)va_arg(vl, const void *)) != NULL) {
            len = va_arg(vl, uint32_t);

            if ((ctx->m + 3 != len) ||
                (in[0] != CCARD_ALGO_LINEAR) ||
                (in[1] != ctx->hf)) {

                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            bctx = lnr_cnt_init(in, ctx->m, ctx->hf);
            lnr_cnt_merge(ctx, bctx, NULL);
            lnr_cnt_fini(bctx);
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int lnr_cnt_reset(lnr_cnt_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->count = ctx->length;
    ctx->err = CCARD_OK;
    memset(ctx->M, 0, ctx->m);

    return 0;
}

int lnr_cnt_fini(lnr_cnt_ctx_t *ctx)
{
    if (ctx) {
        free(ctx);
        return 0;
    }

    return -1;
}

int lnr_cnt_errnum(lnr_cnt_ctx_t *ctx)
{
    if (ctx) {
        return ctx->err;
    }

    return CCARD_ERR_INVALID_CTX;
}

const char *lnr_cnt_errstr(int err)
{
    static const char *msg[] = {
        "No error",
        "Invalid algorithm context",
        "Merge bitmap failed",
        NULL
    };

    if (-err >= 0 && -err < (int)(sizeof(msg) / sizeof(msg[0]) - 1)) {
        return msg[-err];
    }

    return "Invalid error number";
}

// vi:ft=c ts=4 sw=4 fdm=marker et

