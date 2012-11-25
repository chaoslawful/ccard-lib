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

// vi:ft=c ts=4 sw=4 fdm=marker et

