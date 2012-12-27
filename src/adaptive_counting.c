#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "murmurhash.h"
#include "lookup3hash.h"
#include "adaptive_counting.h"

struct adp_cnt_ctx_s {
    int err;
    uint8_t k;
    uint32_t m;
    double Ca;
    uint32_t Rsum;
    uint32_t b_e;
    uint8_t hf;
    uint8_t M[1];
};

/**
 * Gamma function computed using SciLab
 *
 *  ((gamma(-(m.^(-1))).* ( (1-2.^(m.^(-1)))./log(2) )).^(-m)).*m
 *
 * which is a_m in page 5 of the paper "LogLog Counting of Large
 * Cardinalities".
 * */
static const double alpha[] = {
    0,
    0.44567926005415,
    1.2480639342271,
    2.8391255240079,
    6.0165231584811,
    12.369319965552,
    25.073991603109,
    50.482891762521,
    101.30047482549,
    202.93553337953,
    406.20559693552,
    812.74569741657,
    1625.8258887309,
    3251.9862249084,
    6504.3071471860,
    13008.949929672,
    26018.222470181,
    52036.684135280,
    104073.41696276,
    208139.24771523,
    416265.57100022,
    832478.53851627,
    1669443.2499579,
    3356902.8702907,
    6863377.8429508,
    11978069.823687,
    31333767.455026,
    52114301.457757,
    72080129.928986,
    68945006.880409,
    31538957.552704,
    3299942.4347441
};

/**
 * Switching empty bucket ratio
 */
static const double B_s = 0.051;

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

adp_cnt_ctx_t *adp_cnt_raw_init(const void *obuf, uint32_t len_or_k, uint8_t hf)
{
    adp_cnt_ctx_t *ctx;
    uint8_t *buf = (uint8_t *)obuf;

    if (len_or_k == 0) {
        // invalid buffer length or k
        return NULL;
    }

    if (buf) {
        // initial bitmap was given
        uint32_t i;
        uint8_t k = num_of_trail_zeros(len_or_k);

        if (len_or_k != (uint32_t)(1 << k)) {
            // invalid buffer size, its length must be a power of 2
            return NULL;
        }

        ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t) + len_or_k - 1);
        ctx->err = CCARD_OK;
        memcpy(ctx->M, buf, len_or_k);
        ctx->m = len_or_k;
        ctx->k = k;
        ctx->Ca = alpha[k];
        ctx->Rsum = 0;
        ctx->hf = hf;
        ctx->b_e = ctx->m;
        for(i = 0; i < len_or_k; ++i) {
            ctx->Rsum += buf[i];
            if (buf[i] > 0) {
                ctx->b_e--;
            }
        }
    } else {
        // k was given
        if (len_or_k >= sizeof(alpha) / sizeof(alpha[0])) {
            // exceeded maximum k
            return NULL;
        }

        ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t) + (1 << len_or_k) - 1);
        ctx->err = CCARD_OK;
        ctx->m = 1 << len_or_k;
        ctx->k = (uint8_t)len_or_k;
        ctx->Ca = alpha[len_or_k];
        ctx->Rsum = 0;
        ctx->hf = hf;
        ctx->b_e = ctx->m;
        memset(ctx->M, 0, ctx->m);
    }

    return ctx;
}

adp_cnt_ctx_t *adp_cnt_init(const void *obuf, uint32_t len_or_k, uint8_t hf)
{
    uint8_t *buf = (uint8_t *)obuf;

    if (buf) {
        uint8_t k = num_of_trail_zeros(len_or_k);

        if (buf[0] != CCARD_ALGO_ADAPTIVE ||
            buf[1] != hf ||
            buf[2] != k) {

            // counting algorithm, hash function or length not match
            return NULL;
        }

        return adp_cnt_raw_init(buf + 3, len_or_k, hf);
    }

    return adp_cnt_raw_init(NULL, len_or_k, hf);
}

int64_t adp_cnt_card_loglog(adp_cnt_ctx_t *ctx)
{
    double Ravg;

    if (!ctx) {
        return -1;
    }

    Ravg = ctx->Rsum / (double)ctx->m;
    ctx->err = CCARD_OK;
    return (int64_t)(ctx->Ca * pow(2, Ravg));
}

int64_t adp_cnt_card(adp_cnt_ctx_t *ctx)
{
    double B = ctx->b_e / (double)ctx->m;

    if (!ctx) {
        return -1;
    }

    if (B >= B_s) {
        ctx->err = CCARD_OK;
        return (int64_t)round((-(double)ctx->m) * log(B));
    }

    return adp_cnt_card_loglog(ctx);
}

int adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len)
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
            /* default to use lookup3 hash function */
            x = lookup3ycs64_2((const char *)buf);
            hl = 64;
    }

    j = x >> (hl - ctx->k);
    r = (uint8_t)(num_of_trail_zeros(x << (ctx->k + 64 - hl)) - (ctx->k + 64 - hl) + 1);
    if (ctx->M[j] < r) {
        ctx->Rsum += r - ctx->M[j];
        if (ctx->M[j] == 0) {
            ctx->b_e--;
        }
        ctx->M[j] = r;

        modified = 1;
    }

    ctx->err = CCARD_OK;
    return modified;
}

int adp_cnt_get_raw_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || !len || (out && *len < ctx->m)) {
        return -1;
    }

    if(out) {
        memcpy(out, ctx->M, ctx->m);
    }
    *len = ctx->m;

    return 0;
}

int adp_cnt_get_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    /*
     +--------------+---------+------------------------------+-----------+
     | algorithm[1] | hash[1] | bitmap length(base-2 log)[1] | bitmap[n] |
     +--------------+---------+------------------------------+-----------+
     */
    uint8_t algo = CCARD_ALGO_ADAPTIVE;
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || !len || (out && *len < ctx->m + 3)) {
        return -1;
    }

    if (out) {
        out[0] = algo;
        out[1] = ctx->hf;
        out[2] = ctx->k;
        memcpy(out + 3, ctx->M, ctx->m);
    }
    *len = ctx->m + 3;

    return 0;
}

int adp_cnt_merge(adp_cnt_ctx_t *ctx, adp_cnt_ctx_t *tbm, ...)
{
    va_list vl;
    adp_cnt_ctx_t *bm;
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
                ctx->Rsum += tbm->M[i] - ctx->M[i];
                if (ctx->M[i] == 0) {
                    ctx->b_e--;
                }
                ctx->M[i] = tbm->M[i];
            }
        }

        va_start(vl, tbm);
        while ((bm = va_arg(vl, adp_cnt_ctx_t *)) != NULL) {
            if ((tbm->m != ctx->m) || (tbm->hf != ctx->hf)) {
                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            for (i = 1; i < ctx->m; i++) {
                if (bm->M[i] > ctx->M[i]) {
                    ctx->Rsum += bm->M[i] - ctx->M[i];
                    ctx->M[i] = bm->M[i];
                }
            }
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int adp_cnt_merge_raw_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    va_list vl;
    uint8_t *in;
    adp_cnt_ctx_t *bctx;

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

        bctx = adp_cnt_raw_init(in, ctx->m, ctx->hf);
        adp_cnt_merge(ctx, bctx, NULL);
        adp_cnt_fini(bctx);

        va_start(vl, len);
        while ((in = (uint8_t *)va_arg(vl, const void *)) != NULL) {
            len = va_arg(vl, uint32_t);

            if (ctx->m != len) {
                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            bctx = adp_cnt_raw_init(in, ctx->m, ctx->hf);
            adp_cnt_merge(ctx, bctx, NULL);
            adp_cnt_fini(bctx);
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int adp_cnt_merge_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    va_list vl;
    uint8_t *in;
    adp_cnt_ctx_t *bctx;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        in = (uint8_t *)buf;
        /* Cannot merge bitmap of different sizes,
        different hash functions or different algorithms */
        if ((ctx->m + 3 != len) ||
            (in[0] != CCARD_ALGO_ADAPTIVE) ||
            (in[1] != ctx->hf)) {

            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        bctx = adp_cnt_init(in, ctx->m, ctx->hf);
        adp_cnt_merge(ctx, bctx, NULL);
        adp_cnt_fini(bctx);

        va_start(vl, len);
        while ((in = (uint8_t *)va_arg(vl, const void *)) != NULL) {
            len = va_arg(vl, uint32_t);

            if ((ctx->m + 3 != len) ||
                (in[0] != CCARD_ALGO_ADAPTIVE) ||
                (in[1] != ctx->hf)) {

                ctx->err = CCARD_ERR_MERGE_FAILED;
                return -1;
            }

            bctx = adp_cnt_init(in, ctx->m, ctx->hf);
            adp_cnt_merge(ctx, bctx, NULL);
            adp_cnt_fini(bctx);
        }
        va_end(vl);
    }

    ctx->err = CCARD_OK;
    return 0;
}

int adp_cnt_reset(adp_cnt_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->err = CCARD_OK;
    ctx->Rsum = 0;
    memset(ctx->M, 0, ctx->m);
    ctx->b_e = ctx->m;

    return 0;
}

int adp_cnt_fini(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        free(ctx);
        return 0;
    }

    return -1;
}

int adp_cnt_errnum(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        return ctx->err;
    }

    return CCARD_ERR_INVALID_CTX;
}

const char *adp_cnt_errstr(int err)
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

