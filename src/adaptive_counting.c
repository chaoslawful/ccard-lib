#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <alloca.h>
#include <limits.h>
#include <math.h>
#include "murmurhash.h"
#include "lookup3hash.h"
#include "adaptive_counting.h"

struct adp_cnt_ctx_s {
    int err;
    uint8_t k;          /* log2(total buckets) */
    uint8_t hf;         /* hash function id */
    uint8_t sidx_len;   /* bucket index length */
    double Ca;          /* bias correction coefficient */
    uint32_t m;         /* total buckets */
    uint32_t Rsum;      /* sum of non-empty buckets */
    uint32_t b_e;       /* number of empty buckets */
    uint32_t bmp_len;   /* actual bitmap length */
    uint8_t *M;         /* pointer to buckets array */
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
 * algorithm-switching empty bucket ratio
 */
static const double B_s = 0.051;

static uint8_t
num_of_trail_zeros(uint64_t i)
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

static int
sparse_bisect_search(adp_cnt_ctx_t *ctx, int bkt_no)
{
    int step = ctx->sidx_len + 1; /* B[1] and IDX[d] */
    int cur_bkts = (ctx->bmp_len - 1) / step; /* current buckets in sparse
                                                 array */
    int begin = 0, end = cur_bkts, mid, off, idx;

    /* bisect-search the given bucket in sparse array */
    while(begin <= end && begin < cur_bkts && end >= 0) {
        mid = (begin + end) >> 1;
        off = mid * step + 1;
        idx = sparse_bytes_to_int(ctx->M, off + 1, ctx->sidx_len);
        if(bkt_no == idx) {
            return off;
        } else if(bkt_no < idx) {
            end = mid - 1;
        } else {
            begin = mid + 1;
        }
    }

    return -1;
}

static int
sparse_should_use_normal_bitmap(adp_cnt_ctx_t *ctx, uint32_t used_bkts)
{
    return (used_bkts + 1) * (ctx->sidx_len + 1) >= ctx->m;
}

static void
sparse_insert_bucket(adp_cnt_ctx_t *ctx, int bkt_no, uint8_t bkt_val)
{
    int step = ctx->sidx_len + 1;
    int idx;
    uint32_t off;

    /* extend sparse array for one more bucket */
    ctx->M = realloc(ctx->M, ctx->bmp_len + step);

    /* search insertion position */
    for(off = 1; off < ctx->bmp_len; off += step) {
        idx = sparse_bytes_to_int(ctx->M, off + 1, ctx->sidx_len);
        if(idx > bkt_no) {
            break;
        }
    }
    if(off < ctx->bmp_len) {
        /* insertion position is between original buckets, make room for
         * inserted bucket */
        memmove(ctx->M + off + step, ctx->M + off, ctx->bmp_len - off);
    }

    /* insert new bucket */
    ctx->M[off] = bkt_val;
    sparse_int_to_bytes(ctx->M, off + 1, ctx->sidx_len, bkt_no);

    ctx->bmp_len += step;
}

static void
sparse_to_normal_bitmap(adp_cnt_ctx_t *ctx)
{
    int idx;
    uint32_t i, step = ctx->sidx_len + 1;
    uint8_t *bmp = calloc(sizeof(uint8_t), ctx->m);

    /* convert sparse format to normal format */
    for(i = 1; i < ctx->bmp_len; i += step) {
        idx = sparse_bytes_to_int(ctx->M, i + 1, ctx->sidx_len);
        bmp[idx] = ctx->M[i];
    }

    /* replace sparse bucket array to normal one */
    free(ctx->M);
    ctx->bmp_len = ctx->m;
    ctx->M = bmp;
}

/**
 * Verify context and given bitmap has identical total bucket number
 *
 * @param[in] ctx Context to be merged to. Must be non-NULL.
 * @param[in] is_raw 1 if given bitmap is in raw format, 0 if in external
 * format.
 * @param[in] buf The bitmap to be merged. Must be non-NULL.
 * @param[in] len Length of the bitmap.
 * @retval -1 Verification failed, the given bitmap can't be merged to ctx.
 * @retval 0 Verification success and there exists at least 1 normal bitmap.
 * @retval 1 Verification success and there are only sparse bitmaps.
 * */
static int
unified_bitmap_verify(adp_cnt_ctx_t *ctx, int is_raw,
                      const void *buf, uint32_t len)
{
    int rc;
    const uint8_t *in = buf;
    uint32_t m;

    assert(ctx && buf);

    if(!is_raw) {
        /* bitmap is not in raw format, check header first */
        if(in[0] != CCARD_ALGO_ADAPTIVE
           || in[1] != ctx->hf
           || in[2] != ctx->k
           || len <= 3) {
            return -1;
        }

        /* strip out header bytes */
        in += 3;
        len -= 3;
    }

    if(IS_SPARSE_BMP(in)) {
        m = 1 << K_FROM_ID(in[0]);
        rc = 1;
    } else {
        m = len;
        rc = 0;
    }
    if(ctx->m != m) {
        return -1;
    }

    return rc;
}

/**
 * Check if there are bitmaps in normal format among all bitmaps to be merged.
 *
 * @param[in] ctx Context to be merged to. Must be non-NULL.
 * @param[in] buf_cnt Number of raw bitmaps to be merged.
 * @param[in] pbuf Pointer to the array of raw bitmap pointers.
 * @param[in] plen Pointer to the array of raw bitmap lengths.
 * @retval 0 Verification success and there exists at least 1 normal bitmap.
 * @retval 1 Verification success and there are only sparse bitmaps.
 * */
static int
is_there_normal_raw_bitmap(adp_cnt_ctx_t *ctx, int buf_cnt,
                           const uint8_t **pbuf, uint32_t *plen)
{
    int rc;
    int i;
    int sparse_only = 1;

    assert(ctx);

    for(i = 0; i < buf_cnt; i++) {
        rc = unified_bitmap_verify(ctx, 1, pbuf[i], plen[i]);
        if(rc == 0) {
            sparse_only = 0;
        }
    }

    if(sparse_only) {
        return 1;
    }
    return 0;
}

/**
 * Count distinct buckets among all given sparse bitmaps
 *
 * @note It is done by n-way merge all bucket arrays
 * */
static uint32_t
sparse_count_distinct_buckets(adp_cnt_ctx_t *ctx, int buf_cnt,
                              const uint8_t **pbuf, uint32_t *plen)
{
    int i;
    int total_bkts;
    int idx;
    int last_idx;
    int min_idx;
    int min_est_no;
    uint32_t *offs = (uint32_t *)alloca(sizeof(uint32_t) * buf_cnt);

    /* ignore initial ID byte in all sparse bitmaps */
    for(i = 0; i < buf_cnt; i++) {
        offs[i] = 1;
    }

    /* n-way merge to count distinct buckets */
    total_bkts = 0;
    min_est_no = 0;
    last_idx = -1;
    while(min_est_no != -1) {
        min_idx = INT_MAX;
        min_est_no = -1;

        /* find minimum bucket index among heads of sparse bucket arrays */
        for(i = 0; i < buf_cnt; i++) {
            if(offs[i] < plen[i]) {
                idx = sparse_bytes_to_int(pbuf[i], offs[i] + 1, ctx->sidx_len);
                if(idx < min_idx) {
                    min_idx = idx;
                    min_est_no = i;
                }
            }
        }

        if(min_est_no != -1) {
            /* advance the head of merged sparse bucket array */
            offs[min_est_no] += ctx->sidx_len + 1;
            /* increase count of distinct bucket when new index occured*/
            if(last_idx != min_idx) {
                total_bkts++;
                last_idx = min_idx;
            }
        }
    }

    return total_bkts;
}

/**
 * Merge all given sparse/normal bitmaps to a normal bitmap
 *
 * @note dbm should be zero'd out before call this routine, otherwise the
 * result would be unexpected
 * */
static void
merge_to_normal_bmp(uint8_t *dbm, adp_cnt_ctx_t *ctx, int buf_cnt,
                    const uint8_t **pbuf, uint32_t *plen)
{
    int step = ctx->sidx_len + 1;
    int i;
    uint32_t j;

    for(i = 0; i < buf_cnt; i++) {
        if(IS_SPARSE_BMP(pbuf[i])) {
            /* merge sparse bitmap */
            for(j = 1; j < plen[i]; j += step) {
                uint8_t r = pbuf[i][j];
                int idx = sparse_bytes_to_int(pbuf[i], j + 1, ctx->sidx_len);
                if(dbm[idx] < r) {
                    dbm[idx] = r;
                }
            }
        } else {
            /* merge normal bitmap */
            for(j = 0; j < plen[i]; j++) {
                if(dbm[j] < pbuf[i][j]) {
                    dbm[j] = pbuf[i][j];
                }
            }
        }
    }
}

/**
 * Merge all given sparse bitmaps to a sparse bitmap
 *
 * @note dbm should be zero'd out before call this routine, otherwise the
 * result would be unexpected
 * */
static void
merge_to_sparse_bmp(uint8_t *dbm, adp_cnt_ctx_t *ctx,
                    int buf_cnt, const uint8_t **pbuf, uint32_t *plen)
{
    int i;
    int idx;
    int min_idx;
    int min_est_no;
    uint8_t min_r;
    int last_idx;
    int step = ctx->sidx_len + 1;
    int off;
    uint32_t *offs = (uint32_t *)alloca(sizeof(uint32_t) * buf_cnt);

    /* generate sparse bitmap ID */
    off = 1;
    dbm[0] = MAKE_SPARSE_ID(ctx->k);

    /* ignore initial ID byte in all sparse bitmaps */
    for(i = 0; i < buf_cnt; i++) {
        offs[i] = 1;
    }

    /* n-way merge sparse bitmaps */
    min_est_no = 0;
    min_r = 0;
    last_idx = -1;
    while(min_est_no != -1) {
        min_idx = INT_MAX;
        min_est_no = -1;

        /* find minimum bucket index among heads of sparse bucket arrays */
        for(i = 0; i < buf_cnt; i++) {
            if(offs[i] < plen[i]) {
                idx = sparse_bytes_to_int(pbuf[i], offs[i] + 1, ctx->sidx_len);
                if(idx < min_idx) {
                    min_idx = idx;
                    min_est_no = i;
                    min_r = pbuf[i][offs[i]];
                }
            }
        }

        if(min_est_no != -1) {
            /* insert bucket index */
            if(last_idx != min_idx) {
                if(last_idx != -1) {
                    off += step;
                }
                sparse_int_to_bytes(dbm, off + 1, ctx->sidx_len, min_idx);
                last_idx = min_idx;
            }
            /* merge bucket value */
            if(dbm[off] < min_r) {
                dbm[off] = min_r;
            }
            /* advance the head of merged sparse bucket array */
            offs[min_est_no] += step;
        }
    }
}

/**
 * Update estimator state in context according to bucket array data
 *
 * @note Context must be initialized with correct bucket array data first
 * */
static void
update_estimator_state(adp_cnt_ctx_t *ctx, int init)
{
    uint32_t i;

    ctx->sidx_len = (ctx->k + 7) / 8; /* =(int)ceil(k/8.0) */
    ctx->Ca = alpha[ctx->k];
    ctx->Rsum = 0;
    ctx->b_e = ctx->m;

    if(!init) {
        if(IS_SPARSE_BMP(ctx->M)) {
            /* skip ID byte and accumulate all sparse buckets */
            for(i = 1; i < ctx->bmp_len; i += ctx->sidx_len + 1) {
                ctx->Rsum += ctx->M[i];
                ctx->b_e--;
            }
        } else {
            /* traverse all buckets and accumulate non-empty ones */
            for(i = 0; i < ctx->bmp_len; i++) {
                if (ctx->M[i] > 0) {
                    ctx->Rsum += ctx->M[i];
                    ctx->b_e--;
                }
            }
        }
    }
}

/**
 * Merge all given raw bitmaps and replace bitmap in current context
 *
 * @note The given raw bitmap list should contains current context bitmap if it
 * should also be merged
 * */
static int
aux_merge_raw_bytes(adp_cnt_ctx_t *ctx, int buf_cnt,
                    const uint8_t **pbuf, uint32_t *plen)
{
    int rc;
    int gen_normal = 0;
    uint8_t *dbm = NULL;
    uint32_t dlen;
    uint32_t bkts = 0;

    rc = is_there_normal_raw_bitmap(ctx, buf_cnt, pbuf, plen);
    if(rc == 0) {
        /* there exists at least 1 normal bitmap, merge to normal format */
        dlen = ctx->m;
        gen_normal = 1;
    } else {
        /* there are only sparse bitmaps */
        bkts = sparse_count_distinct_buckets(ctx, buf_cnt, pbuf, plen);
        if(!sparse_should_use_normal_bitmap(ctx, bkts)) {
            /* merge to sparse format has less memory overhead */
            dlen = bkts * (ctx->sidx_len + 1) + 1; /* buckets + ID */
            gen_normal = 0;
        } else {
            /* merge to normal format */
            dlen = ctx->m;
            gen_normal = 1;
        }
    }

    dbm = (uint8_t *)calloc(sizeof(uint8_t), dlen);
    if(gen_normal) {
        merge_to_normal_bmp(dbm, ctx, buf_cnt, pbuf, plen);
    } else {
        merge_to_sparse_bmp(dbm, ctx, buf_cnt, pbuf, plen);
    }

    /* replace context bitmap with merged one and update estimator state */
    free(ctx->M);
    ctx->M = dbm;
    ctx->bmp_len = dlen;
    update_estimator_state(ctx, 0);

    ctx->err = CCARD_OK;
    return 0;
}

adp_cnt_ctx_t *
adp_cnt_raw_init(const void *obuf, uint32_t len_or_k, uint8_t opt)
{
    adp_cnt_ctx_t *ctx;
    uint8_t *buf = (uint8_t *)obuf;

    if (len_or_k == 0) {
        /* invalid buffer length or k */
        return NULL;
    }

    if (buf) {
        /* initial bitmap was given */
        uint8_t k;
        uint32_t m;

        if(IS_SPARSE_BMP(buf)) {
            /*
             * initial bitmap is sparse one, calculate real bitmap size.
             *
             * the sparse bitmap format is:
             *
             *  +-------+-------+---------+-------+---------+-----+
             *  | ID[1] | B0[1] | IDX0[d] | B1[1] | IDX1[d] | ... |
             *  +-------+-------+---------+-------+---------+-----+
             *
             * where ID is 0x80|k, B* is non-empty bucket values (always >0)
             * and IDX* is corresponding bucket index (var-len little-endian
             * bytes, equal to ceil(k/8)).
             *
             * if there're no non-empty buckets, sparse bitmap has only the ID
             * byte.
             */
            k = K_FROM_ID(buf[0]);
            m = (uint32_t)(1 << k);
        } else {
            /* initial bitmap is normal one */
            k = num_of_trail_zeros(len_or_k);
            m = len_or_k;
            if (m != (uint32_t)(1 << k)) {
                /* invalid buffer size, its length must be a power of 2 */
                return NULL;
            }
        }

        if (k >= sizeof(alpha) / sizeof(alpha[0])) {
            /* exceeded maximum k */
            return NULL;
        }

        ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t));
        ctx->err = CCARD_OK;
        ctx->m = m;
        ctx->k = k;
        ctx->bmp_len = len_or_k;
        ctx->M = malloc(ctx->bmp_len);
        memcpy(ctx->M, buf, ctx->bmp_len);
        ctx->hf = HF(opt);

        update_estimator_state(ctx, 0);
    } else {
        /* only k was given */
        uint8_t k = len_or_k;
        if (k >= sizeof(alpha) / sizeof(alpha[0])) {
            /* exceeded maximum k */
            return NULL;
        }

        if(opt & CCARD_OPT_SPARSE) {
            /* create sparse bitmap with only ID byte*/
            ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t));
            ctx->bmp_len = 1;
            ctx->M = malloc(ctx->bmp_len);
            ctx->M[0] = MAKE_SPARSE_ID(k);
        } else {
            /* create normal bitmap */
            ctx = (adp_cnt_ctx_t *)malloc(sizeof(adp_cnt_ctx_t));
            ctx->bmp_len = 1 << k;
            ctx->M = malloc(ctx->bmp_len);
            memset(ctx->M, 0, ctx->bmp_len);
        }

        ctx->err = CCARD_OK;
        ctx->m = 1 << k;
        ctx->k = k;
        ctx->hf = HF(opt);

        update_estimator_state(ctx, 1);
    }

    return ctx;
}

adp_cnt_ctx_t *
adp_cnt_init(const void *obuf, uint32_t len_or_k, uint8_t opt)
{
    uint8_t *buf = (uint8_t *)obuf;

    if (buf) {
        uint8_t k = num_of_trail_zeros(len_or_k);

        if (buf[0] != CCARD_ALGO_ADAPTIVE ||
            buf[1] != HF(opt) ||
            buf[2] != k) {

            /* counting algorithm, hash function or length not match */
            return NULL;
        }

        return adp_cnt_raw_init(buf + 3, len_or_k, opt);
    }

    return adp_cnt_raw_init(NULL, len_or_k, opt);
}

int64_t
adp_cnt_card_loglog(adp_cnt_ctx_t *ctx)
{
    double Ravg;

    if (!ctx) {
        return -1;
    }

    Ravg = ctx->Rsum / (double)ctx->m;
    ctx->err = CCARD_OK;
    return (int64_t)(ctx->Ca * pow(2, Ravg));
}

int64_t
adp_cnt_card(adp_cnt_ctx_t *ctx)
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

int
adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len)
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
    r = (uint8_t)(num_of_trail_zeros(x << (ctx->k + 64 - hl)) - (ctx->k + 64 -
                  hl) + 1);

    if(IS_SPARSE_BMP(ctx->M)) {
        /* update sparse bucket counter */
        int off = sparse_bisect_search(ctx, j);

        if(off != -1) {
            /* the bucket to be updated already exists, no need to decrease
             * empty bucket counter */
            if(ctx->M[off] < r) {
                ctx->Rsum += r - ctx->M[off];
                ctx->M[off] = r;
                modified = 1;
            }
            return modified;
        }

        if(!sparse_should_use_normal_bitmap(ctx, ctx->m - ctx->b_e)) {
            /* still use sparse format to insert new bucket */
            sparse_insert_bucket(ctx, j, r);
            ctx->Rsum += r;
            ctx->b_e--;
            return 1;
        }

        /* convert sparse buckets to normal format, fallback to CONT */
        sparse_to_normal_bitmap(ctx);
    }

    /* CONT: update normal bucket counter */
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

int
adp_cnt_get_raw_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || !len || (out && *len < ctx->bmp_len)) {
        return -1;
    }

    if(out) {
        memcpy(out, ctx->M, ctx->bmp_len);
    }
    *len = ctx->bmp_len;

    return 0;
}

int
adp_cnt_get_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len)
{
    /*
     +--------------+---------+------------------------------+-----------+
     | algorithm[1] | hash[1] | bitmap length(base-2 log)[1] | bitmap[n] |
     +--------------+---------+------------------------------+-----------+
     */
    uint8_t algo = CCARD_ALGO_ADAPTIVE;
    uint8_t *out = (uint8_t *)buf;

    if (!ctx || !len || (out && *len < ctx->bmp_len + 3)) {
        return -1;
    }

    if (out) {
        out[0] = algo;
        out[1] = ctx->hf;
        out[2] = ctx->k;
        memcpy(out + 3, ctx->M, ctx->bmp_len);
    }
    *len = ctx->bmp_len + 3;

    return 0;
}

int
adp_cnt_merge(adp_cnt_ctx_t *ctx, adp_cnt_ctx_t *tbm, ...)
{
    int rc;
    va_list vl;
    adp_cnt_ctx_t *bm;

    if (!ctx) {
        return -1;
    }

    if (tbm) {
        int invalid = 0;
        int buf_cnt;
        const uint8_t **pbuf;
        uint32_t *plen;

        /* count number of estimators and validate them */
        buf_cnt = 2; /* current context and the 1st estimator in args */
        rc = unified_bitmap_verify(ctx, 1, tbm->M, tbm->bmp_len);
        if(rc == -1 || ctx->hf != tbm->hf) {
            invalid = 1;
        } else {
            va_start(vl, tbm);
            while ((bm = va_arg(vl, adp_cnt_ctx_t *)) != NULL) {
                rc = unified_bitmap_verify(ctx, 1, bm->M, bm->bmp_len);
                if(rc == -1 || ctx->hf != bm->hf) {
                    invalid = 1;
                    break;
                }
                buf_cnt++;
            }
            va_end(vl);
        }

        if(invalid) {
            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        pbuf = (const uint8_t **)alloca(sizeof(const uint8_t *) * buf_cnt);
        plen = (uint32_t *)alloca(sizeof(uint32_t) * buf_cnt);

        /* initialize buffer array */
        buf_cnt = 2;
        pbuf[0] = ctx->M;
        plen[0] = ctx->bmp_len;
        pbuf[1] = tbm->M;
        plen[1] = tbm->bmp_len;

        va_start(vl, tbm);
        while ((bm = va_arg(vl, adp_cnt_ctx_t *)) != NULL) {
            pbuf[buf_cnt] = bm->M;
            plen[buf_cnt] = bm->bmp_len;
            buf_cnt++;
        }
        va_end(vl);

        rc = aux_merge_raw_bytes(ctx, buf_cnt, pbuf, plen);
    } else {
        ctx->err = CCARD_OK;
        rc = 0;
    }

    return rc;
}

int
adp_cnt_merge_raw_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    int rc;
    va_list vl;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        int invalid = 0;
        int buf_cnt;
        const void *in_buf;
        uint32_t in_len;
        const uint8_t **pbuf;
        uint32_t *plen;

        /* count number of buffers and validate them */
        buf_cnt = 2; /* current context and the 1st buffer in args */
        rc = unified_bitmap_verify(ctx, 1, buf, len);
        if(rc == -1) {
            invalid = 1;
        } else {
            va_start(vl, len);
            while((in_buf = va_arg(vl, const void *)) != NULL) {
                in_len = va_arg(vl, uint32_t);
                rc = unified_bitmap_verify(ctx, 1, in_buf, in_len);
                if(rc == -1) {
                    invalid = 1;
                    break;
                }
                buf_cnt++;
            }
            va_end(vl);
        }

        if(invalid) {
            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        pbuf = (const uint8_t **)alloca(sizeof(const uint8_t *) * buf_cnt);
        plen = (uint32_t *)alloca(sizeof(uint32_t) * buf_cnt);

        /* initialize buffer array */
        buf_cnt = 2;
        pbuf[0] = ctx->M;
        plen[0] = ctx->bmp_len;
        pbuf[1] = buf;
        plen[1] = len;

        va_start(vl, len);
        while((in_buf = va_arg(vl, const void *)) != NULL) {
            in_len = va_arg(vl, uint32_t);
            pbuf[buf_cnt] = in_buf;
            plen[buf_cnt] = in_len;
            buf_cnt++;
        }
        va_end(vl);

        rc = aux_merge_raw_bytes(ctx, buf_cnt, pbuf, plen);
    } else {
        ctx->err = CCARD_OK;
        rc = 0;
    }

    return rc;
}

int
adp_cnt_merge_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...)
{
    int rc;
    va_list vl;

    if (!ctx) {
        return -1;
    }

    if (buf) {
        int invalid = 0;
        int buf_cnt;
        const void *in_buf;
        uint32_t in_len;
        const uint8_t **pbuf;
        uint32_t *plen;


        /* count number of buffers and validate them */
        buf_cnt = 2; /* current context and the 1st buffer in args */
        rc = unified_bitmap_verify(ctx, 0, buf, len);
        if(rc == -1) {
            invalid = 1;
        } else {
            va_start(vl, len);
            while((in_buf = va_arg(vl, const void *)) != NULL) {
                in_len = va_arg(vl, uint32_t);
                rc = unified_bitmap_verify(ctx, 0, in_buf, in_len);
                if(rc == -1) {
                    invalid = 1;
                    break;
                }
                buf_cnt++;
            }
            va_end(vl);
        }

        if(invalid) {
            ctx->err = CCARD_ERR_MERGE_FAILED;
            return -1;
        }

        pbuf = (const uint8_t **)alloca(sizeof(const uint8_t *) * buf_cnt);
        plen = (uint32_t *)alloca(sizeof(uint32_t) * buf_cnt);

        /* initialize buffer array (strip headers) */
        buf_cnt = 2;
        pbuf[0] = ctx->M;
        plen[0] = ctx->bmp_len;
        pbuf[1] = (const uint8_t *)buf + 3;
        plen[1] = len - 3;

        va_start(vl, len);
        while((in_buf = va_arg(vl, const void *)) != NULL) {
            in_len = va_arg(vl, uint32_t);
            pbuf[buf_cnt] = (const uint8_t *)in_buf + 3;
            plen[buf_cnt] = in_len - 3;
            buf_cnt++;
        }
        va_end(vl);

        rc = aux_merge_raw_bytes(ctx, buf_cnt, pbuf, plen);
    } else {
        ctx->err = CCARD_OK;
        rc = 0;
    }

    return rc;
}

int
adp_cnt_reset(adp_cnt_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->err = CCARD_OK;
    ctx->Rsum = 0;
    ctx->b_e = ctx->m;
    if(IS_SPARSE_BMP(ctx->M)) {
        ctx->M = realloc(ctx->M, 1);
        ctx->M[0] = MAKE_SPARSE_ID(ctx->k);
    } else {
        memset(ctx->M, 0, ctx->m);
    }

    return 0;
}

int
adp_cnt_fini(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        free(ctx->M);
        free(ctx);
        return 0;
    }

    return -1;
}

int
adp_cnt_errnum(adp_cnt_ctx_t *ctx)
{
    if (ctx) {
        return ctx->err;
    }

    return CCARD_ERR_INVALID_CTX;
}

const char *
adp_cnt_errstr(int err)
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

/* vi:ft=c ts=4 sw=4 fdm=marker et
 * */

