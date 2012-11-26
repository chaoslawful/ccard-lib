#include <math.h>
#include "ccard_common.h"
#include "loglog_counting.h"
#include "adaptive_counting.h"
#include "gtest/gtest.h"

/**
 * Tests initialize Adaptive counting context with empty buffer.
 *
 * <p>
 * </p>
 * */
TEST(AdaptiveCounting, InitCtxEmpty) {
    uint32_t m = pow(2, 16);

    adp_cnt_ctx_t *ctx = adp_cnt_init(NULL, 16);

    EXPECT_EQ(CCARD_OK, ctx->super->err);
    EXPECT_EQ(16, ctx->super->k);
    EXPECT_EQ(m, ctx->super->m);
    EXPECT_EQ(26018.222470181, ctx->super->Ca);
    EXPECT_EQ(0, ctx->super->Rsum);
    EXPECT_EQ(0, ctx->super->M[0]);
    EXPECT_EQ(0, ctx->super->M[m - 1]);

    EXPECT_EQ(m, ctx->b_e);

    adp_cnt_fini(ctx);
}

/**
 * Tests initialize Adaptive counting context with bitmap.
 *
 * <p>
 * </p>
 * */
TEST(AdaptiveCounting, InitCtxBitmap) {
    uint32_t m = pow(2, 16);
    uint8_t bitmap[m];
    memset(bitmap, 10, m);

    adp_cnt_ctx_t *ctx = adp_cnt_init(bitmap, m);

    EXPECT_EQ(CCARD_OK, ctx->super->err);
    EXPECT_EQ(16, ctx->super->k);
    EXPECT_EQ(m, ctx->super->m);
    EXPECT_EQ(26018.222470181, ctx->super->Ca);
    EXPECT_EQ(10 * m, ctx->super->Rsum);
    EXPECT_EQ(10, ctx->super->M[0]);
    EXPECT_EQ(10, ctx->super->M[m - 1]);

    EXPECT_EQ(0, ctx->b_e);

    adp_cnt_fini(ctx);
}

/**
 * Tests Adaptive counting.
 *
 * <p>
 * </p>
 * */
TEST(AdaptiveCounting, Counting) {
    int64_t i, esti;
    adp_cnt_ctx_t *ctx = adp_cnt_init(NULL, 16);

    for (i = 0; i < 1000000L; i++) {
        adp_cnt_offer(ctx, &i, sizeof(int));

        if (i % 50000 == 0) {
            esti = adp_cnt_card(ctx);
            printf("B: %f, actual: %u, estimated: %u, error: %.2f%%\n", 
                    ctx->b_e / (double)ctx->super->m, i, esti, fabs((double)(esti - i) / i * 100));
        }
    }

    adp_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

