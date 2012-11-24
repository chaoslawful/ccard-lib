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
    ll_cnt_ctx_t *ctx = ll_cnt_init(NULL, 8);

    EXPECT_EQ(CCARD_OK, ctx->err);
    EXPECT_EQ(8, ctx->k);
    EXPECT_EQ(256, ctx->m);
    EXPECT_EQ(101.30047482549, ctx->Ca);
    EXPECT_EQ(0, ctx->Rsum);
    EXPECT_EQ(0, ctx->M[0]);
    EXPECT_EQ(0, ctx->M[127]);
    EXPECT_EQ(0, ctx->M[255]);

    ll_cnt_fini(ctx);
}

/**
 * Tests initialize LogLog counting context with bitmap.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, InitCtxBitmap) {
    uint8_t bitmap[256];
    int i;
    for (i = 0; i < 256; i++) {
        bitmap[i] = (uint8_t)10;
    }

    ll_cnt_ctx_t *ctx = ll_cnt_init(bitmap, 256);

    EXPECT_EQ(CCARD_OK, ctx->err);
    EXPECT_EQ(8, ctx->k);
    EXPECT_EQ(256, ctx->m);
    EXPECT_EQ(101.30047482549, ctx->Ca);
    EXPECT_EQ(2560, ctx->Rsum);
    EXPECT_EQ(10, ctx->M[0]);
    EXPECT_EQ(10, ctx->M[127]);
    EXPECT_EQ(10, ctx->M[255]);

    ll_cnt_fini(ctx);
}

/**
 * Tests LogLog counting.
 *
 * <p>
 * </p>
 * */
TEST(LogLogCounting, Counting) {
    uint64_t i, esti;
    ll_cnt_ctx_t *ctx = ll_cnt_init(NULL, 16);

    for (i = 0; i < 20000000L; i++) {
        ll_cnt_offer(ctx, &i, sizeof(int));

        if (i % 1000000 == 0) {
            esti = ll_cnt_card(ctx);
            printf("actual: %u, estimated: %u\n", i, esti);
        }
    }

    ll_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et
