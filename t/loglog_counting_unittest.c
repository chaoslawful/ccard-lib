#include <math.h>
#include "ccard_common.h"
#include "loglog_counting.h"
#include "gtest/gtest.h"

/**
 * Tests initialize LogLog counting context with empty buffer.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, InitCtxEmpty) {
    uint32_t m = pow(2, 16);

    ll_cnt_ctx_t *ctx = ll_cnt_init(NULL, 16);

    EXPECT_EQ(CCARD_OK, ctx->err);
    EXPECT_EQ(16, ctx->k);
    EXPECT_EQ(m, ctx->m);
    EXPECT_EQ(26018.222470181, ctx->Ca);
    EXPECT_EQ(0, ctx->Rsum);
    EXPECT_EQ(0, ctx->M[0]);
    EXPECT_EQ(0, ctx->M[m - 1]);

    ll_cnt_fini(ctx);
}

/**
 * Tests initialize LogLog counting context with bitmap.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, InitCtxBitmap) {
    uint32_t m = pow(2, 16);
    uint8_t bitmap[m];
    int i;
    for (i = 0; i < m; i++) {
        bitmap[i] = (uint8_t)10;
    }

    ll_cnt_ctx_t *ctx = ll_cnt_init(bitmap, m);

    EXPECT_EQ(CCARD_OK, ctx->err);
    EXPECT_EQ(16, ctx->k);
    EXPECT_EQ(m, ctx->m);
    EXPECT_EQ(26018.222470181, ctx->Ca);
    EXPECT_EQ(10 * m, ctx->Rsum);
    EXPECT_EQ(10, ctx->M[0]);
    EXPECT_EQ(10, ctx->M[m - 1]);

    ll_cnt_fini(ctx);
}

/**
 * Tests LogLog counting.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, Counting) {
    int64_t i, esti;
    ll_cnt_ctx_t *ctx = ll_cnt_init(NULL, 16);

    for (i = 0; i < 20000000L; i++) {
        ll_cnt_offer(ctx, &i, sizeof(int));

        if (i % 1000000 == 0) {
            esti = ll_cnt_card(ctx);
            printf("actual: %u, estimated: %u, error: %.2f%%\n", i, esti, fabs((double)(esti - i) / i * 100));
        }
    }

    ll_cnt_fini(ctx);
}

/**
 * Tests LogLog merge.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, Merge) {
    int64_t i, esti;
    ll_cnt_ctx_t *ctx = ll_cnt_init(NULL, 16);
    ll_cnt_ctx_t *tbm1 = ll_cnt_init(NULL, 16);
    ll_cnt_ctx_t *tbm2 = ll_cnt_init(NULL, 16);
    ll_cnt_ctx_t *tbm3 = ll_cnt_init(NULL, 16);
    ll_cnt_ctx_t *tbm4 = ll_cnt_init(NULL, 8);

    for (i = 0; i < 20000000L; i++) {
        ll_cnt_offer(ctx, &i, sizeof(uint64_t));
    }
    for (i = 10000000L; i < 30000000L; i++) {
        ll_cnt_offer(tbm1, &i, sizeof(uint64_t));
    }
    for (i = 20000000L; i < 40000000L; i++) {
        ll_cnt_offer(tbm2, &i, sizeof(uint64_t));
    }
    for (i = 30000000L; i < 50000000L; i++) {
        ll_cnt_offer(tbm3, &i, sizeof(uint64_t));
    }

    ll_cnt_merge(ctx, tbm1, tbm2, tbm3, NULL);
    esti = ll_cnt_card(ctx);
    printf("actual: 50000000, estimated: %u, error: %.2f%%\n", esti, fabs((double)(esti - 50000000) / 50000000 * 100));

    //Bitmap with different size will cause error.
    EXPECT_EQ(-1, ll_cnt_merge(ctx, tbm4, NULL));

    ll_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

