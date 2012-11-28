#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "murmurhash.h"
#include "lookup3hash.h"
#include "hyperloglog_counting.h"

struct hll_cnt_ctx_s {
    int err;
    uint8_t log2m;
    uint32_t m;
    double alphaMM;
    uint8_t hf;
    uint8_t M[1];
};

static const double POW_2_32 = pow(2, 32);
static const double NEGATIVE_POW_2_32 = -4294967296.0;

static uint8_t num_of_trail_zeros(uint64_t i)
{
    uint64_t y;
    uint8_t n = 63;

    if (i == 0)
        return 64;

    y = i << 32;    if (y != 0) { n -= 32; i = y; }
    y = i << 16;    if (y != 0) { n -= 16; i = y; }
    y = i << 8;     if (y != 0) { n -= 8; i = y; }
    y = i << 4;     if (y != 0) { n -= 4; i = y; }
    y = i << 2;     if (y != 0) { n -= 2; i = y; }
    return n - (uint8_t)((i << 1) >> 63);
}

hll_cnt_ctx_t* hll_cnt_init(const void *obuf, uint32_t len_or_k, uint8_t hf)
{
    hll_cnt_ctx_t *ctx;
    uint8_t *buf = (uint8_t*)obuf;
    uint8_t log2m = buf ? num_of_trail_zeros(len_or_k) : len_or_k;
    uint32_t m = pow(2, log2m);

    if (len_or_k == 0) {
        // invalid buffer length or k
        return NULL;
    }

    if (buf) {
        // initial bitmap was given
        if (buf[0] != CCARD_ALGO_HYPERLOGLOG || 
            buf[1] != hf ||
            buf[2] != log2m) {

            // counting algorithm, hash function or length not match
            return NULL;
        }

        if (len_or_k != (uint32_t)(1 << log2m)) {
            // invalid buffer size, its length must be a power of 2
            return NULL;
        }

        ctx = (hll_cnt_ctx_t *)malloc(sizeof(hll_cnt_ctx_t) + len_or_k - 1);
        memcpy(ctx->M, &buf[3], len_or_k);
    } else {
        // k was given
        ctx = (hll_cnt_ctx_t *)malloc(sizeof(hll_cnt_ctx_t) + (1 << len_or_k) - 1);
        memset(ctx->M, 0, ctx->m);
    }
    ctx->err = CCARD_OK;
    ctx->log2m = log2m;
    ctx->m = m;
    ctx->hf = hf;

    switch (log2m) {
        case 4:
            ctx->alphaMM = 0.673 * m * m;
            break;
        case 5:
            ctx->alphaMM = 0.697 * m * m;
            break;
        case 6:
            ctx->alphaMM = 0.709 * m * m;
            break;
        default:
            ctx->alphaMM = (0.7213 / (1 + 1.079 / m)) * m * m;
    }

    return ctx;
}

int64_t hll_cnt_card(hll_cnt_ctx_t *ctx)
{
    double sum = 0, estimate, zeros = 0;
    uint32_t j, z;

    if (!ctx) {
        return -1;
    }
    ctx->err = CCARD_OK;

    for (j = 0; j < ctx->m; j++) {
        sum += pow(2, (-1 * ctx->M[j]));
    }

    estimate = ctx->alphaMM * (1 / sum);

    if (estimate <= (5.0 / 2.0) * ctx->m) {
        for (z = 0; z < ctx->m; z++) {
            if (ctx->M[z] == 0) {
                zeros++;
            }
        }
        return (int64_t)round(ctx->m * log(ctx->m / zeros));
    } else if (estimate <= (1.0 / 30.0) * POW_2_32) {
        return (int64_t)round(estimate);
    } else if (estimate > (1.0 / 30.0) * POW_2_32) {
        return (int64_t)round((NEGATIVE_POW_2_32 * log(1.0 - (estimate / POW_2_32))));
    }
}

int hll_cnt_offer(hll_cnt_ctx_t *ctx, const void *buf, uint32_t len)
{
    int modified = 0;
    uint64_t x, j;
    uint8_t r, hl;

    if (!ctx) {
        return -1;
    }

    switch (ctx->hf) {
        case CCARD_HASH_MURMUR:
            x = (uint64_t)murmurhash((void *)buf, len, -1);
            hl = 32;
            break;
        case CCARD_HASH_LOOKUP3:
            x = lookup3ycs64_2((const char *)buf);
            hl = 64;
            break;
        default:
            /* default to use murmurhash function */
            x = (uint64_t)murmurhash((void *)buf, len, -1);
            hl = 32;
    }

    j = x >> (hl - ctx->log2m);
    r = (uint8_t)(num_of_trail_zeros(x << (ctx->log2m + 64 - hl)) - (ctx->log2m + 64 - hl) + 1);
    if (ctx->M[j] < r) {
        ctx->M[j] = r;

        modified = 1;
    }

    ctx->err = CCARD_OK;
    return modified;
}

int hll_cnt_get_bytes(hll_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    /*
     +--------------+---------+------------------------------+-----------+
     | algorithm[1] | hash[1] | bitmap length(base-2 log)[1] | bitmap[n] |
     +--------------+---------+------------------------------+-----------+
     */
    uint8_t algo = CCARD_ALGO_HYPERLOGLOG;
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || (*len < ctx->m + 3)) {
        return -1;
    }

    if (buf) {
        out[0] = algo;
        out[1] = ctx->hf;
        out[2] = ctx->log2m;
        memcpy(&out[3], ctx->M, ctx->m);
    }
    *len = ctx->m + 3;

    return 0;
}

int hll_cnt_merge(hll_cnt_ctx_t *ctx, hll_cnt_ctx_t *tbm, ...)
{
    va_list vl;
    hll_cnt_ctx_t *bm;
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

        for (i = 1; i < ctx->m; i++) {
            if (tbm->M[i] > ctx->M[i]) {
                ctx->M[i] = tbm->M[i];
            }
        }

        va_start(vl, tbm);
        while ((bm = va_arg(vl, hll_cnt_ctx_t *)) != NULL) {
            if ((tbm->m != ctx->m) || (tbm->hf != ctx->hf)) {
                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            for (i = 1; i < ctx->m; i++) {
                if (bm->M[i] > ctx->M[i]) {
                    ctx->M[i] = bm->M[i];
                }
            }
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int hll_cnt_merge_bytes(hll_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    va_list vl;
    uint8_t *in;
    hll_cnt_ctx_t *bctx;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        in = (uint8_t *)buf;
        /* Cannot merge bitmap of different sizes, 
        different hash functions or different algorithms */
        if ((ctx->m + 3 != len) || 
            (in[0] != CCARD_ALGO_HYPERLOGLOG) || 
            (in[1] != ctx->hf)) {

            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        bctx = hll_cnt_init(in, ctx->m, ctx->hf);
        hll_cnt_merge(ctx, bctx, NULL);
        hll_cnt_fini(bctx);

        va_start(vl, len);
        while ((in = (uint8_t *)va_arg(vl, const void *)) != NULL) {
            len = va_arg(vl, uint32_t);

            if ((ctx->m + 3 != len) || 
                (in[0] != CCARD_ALGO_HYPERLOGLOG) || 
                (in[1] != ctx->hf)) {

                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            bctx = hll_cnt_init(in, ctx->m, ctx->hf);
            hll_cnt_merge(ctx, bctx, NULL);
            hll_cnt_fini(bctx);
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int hll_cnt_reset(hll_cnt_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->err = CCARD_OK;
    memset(ctx->M, 0, ctx->m);

    return 0;
}

int hll_cnt_fini(hll_cnt_ctx_t *ctx)
{
    if (ctx) {
        free(ctx);
        return 0;
    }

    return -1;
}

int hll_cnt_errnum(hll_cnt_ctx_t *ctx)
{
    if (ctx) {
        return ctx->err;
    }

    return CCARD_ERR_INVALID_CTX;
}

const char* hll_cnt_errstr(int err)
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

