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
    EXPECT_EQ(0, ctx->b_e);

    adp_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

