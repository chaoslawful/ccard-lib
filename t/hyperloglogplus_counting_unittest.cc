#include "ccard_common.h"
#include "hyperloglogplus_counting.h"
#include "gtest/gtest.h"

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use HyperloglogPlusCounting algorithm
 * </p>
 * */
TEST(HyperloglogPlusCounting, RawCounting)
{
    int rc;
    int64_t i, esti;
    hllp_cnt_ctx_t *ctx = hllp_cnt_raw_init(NULL, 16);
    EXPECT_NE(ctx, (hllp_cnt_ctx_t *)NULL);

    printf("HyperloglogPlus Counting:\n");
    for (i = 1; i <= 500000L; i++) {
        rc = hllp_cnt_offer(ctx, &i, sizeof(int64_t));
        EXPECT_GE(rc, 0);

        if (i % 50000 == 0) {
            esti = hllp_cnt_card(ctx);
            EXPECT_GT(esti, 0);
            printf("actual: %9lu, estimated: %9lu, error: %+7.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, (double)(esti - i) / i * 100);
        }
    }
    printf("\n");

    rc = hllp_cnt_reset(ctx);
    EXPECT_EQ(rc, 0);

    rc = hllp_cnt_fini(ctx);
    EXPECT_EQ(rc, 0);
}

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use HyperloglogPlusCounting algorithm
 * </p>
 * */
TEST(HyperloglogPlusCounting, Counting)
{
    int rc;
    int64_t i, esti;
    hllp_cnt_ctx_t *ctx = hllp_cnt_init(NULL, 16);
    EXPECT_NE(ctx, (hllp_cnt_ctx_t *)NULL);

    printf("HyperloglogPlus Counting:\n");
    for (i = 1; i <= 500000L; i++) {
        rc = hllp_cnt_offer(ctx, &i, sizeof(int64_t));
        EXPECT_GE(rc, 0);

        if (i % 50000 == 0) {
            esti = hllp_cnt_card(ctx);
            EXPECT_GT(esti, 0);
            printf("actual: %9lu, estimated: %9lu, error: %+7.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, (double)(esti - i) / i * 100);
        }
    }
    printf("\n");

    rc = hllp_cnt_reset(ctx);
    EXPECT_EQ(rc, 0);

    rc = hllp_cnt_fini(ctx);
    EXPECT_EQ(rc, 0);
}

TEST(HyperloglogPlusCounting, Deserialize)
{
  hllp_cnt_ctx_t * ctx = hllp_cnt_init(NULL, 16);
  EXPECT_NE(ctx, (hllp_cnt_ctx_t *)NULL);
  uint32_t num_bytes = 0;
  EXPECT_EQ(hllp_cnt_get_bytes(ctx, NULL, &num_bytes), 0);
  uint8_t buf[num_bytes];
  EXPECT_EQ(hllp_cnt_get_bytes(ctx, buf, &num_bytes), 0);
  hllp_cnt_ctx_t * other = hllp_cnt_init(buf, num_bytes);
  EXPECT_NE(other, (hllp_cnt_ctx_t *)NULL);
}
