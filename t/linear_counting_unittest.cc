#include <math.h>
#include "ccard_common.h"
#include "linear_counting.h"
#include "gtest/gtest.h"

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use LinearCounting algorithm
 * </p>
 * */
TEST(LinearCounting, RawCounting)
{
    int64_t i, esti;
    lnr_cnt_ctx_t *ctx1 = lnr_cnt_raw_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *ctx2 = lnr_cnt_raw_init(NULL, 16, CCARD_HASH_LOOKUP3);

    printf("Linear Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        lnr_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = lnr_cnt_card(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    lnr_cnt_reset(ctx1);

    printf("Linear Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        lnr_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = lnr_cnt_card(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    lnr_cnt_reset(ctx2);

    lnr_cnt_fini(ctx2);
    lnr_cnt_fini(ctx1);
}

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use LinearCounting algorithm
 * </p>
 * */
TEST(LinearCounting, Counting)
{
    int64_t i, esti;
    lnr_cnt_ctx_t *ctx1 = lnr_cnt_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *ctx2 = lnr_cnt_init(NULL, 16, CCARD_HASH_LOOKUP3);

    printf("Linear Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        lnr_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = lnr_cnt_card(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    lnr_cnt_reset(ctx1);

    printf("Linear Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        lnr_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = lnr_cnt_card(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    lnr_cnt_reset(ctx2);

    lnr_cnt_fini(ctx2);
    lnr_cnt_fini(ctx1);
}

/**
 * Tests serialize, unserialize and merge.
 *
 * <ol>
 * <li>Current context contains 1 to 20000</li>
 * <li>Tbm1 that contains 10000 to 30000 be serialized as buf1</li>
 * <li>Tbm2 that contains 20000 to 40000 be serialized as buf2</li>
 * <li>Merges buf1 and buf2 into current context</li>
 * </ol>
 * */
TEST(LinearCounting, RawMerge)
{
    int64_t i, esti;
    lnr_cnt_ctx_t *ctx = lnr_cnt_raw_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *tbm1 = lnr_cnt_raw_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *tbm2 = lnr_cnt_raw_init(NULL, 16, CCARD_HASH_MURMUR);
    int32_t m = 1<<16;
    uint8_t buf1[m + 3], buf2[m + 3];
    uint32_t len1 = m + 3, len2 = m + 3;

    for (i = 1; i <= 20000L; i++) {
        lnr_cnt_offer(ctx, &i, sizeof(uint64_t));
    }
    for (i = 10000L; i <= 30000L; i++) {
        lnr_cnt_offer(tbm1, &i, sizeof(uint64_t));
    }
    lnr_cnt_get_raw_bytes(tbm1, buf1, &len1);
    for (i = 20000L; i <= 40000L; i++) {
        lnr_cnt_offer(tbm2, &i, sizeof(uint64_t));
    }
    lnr_cnt_get_raw_bytes(tbm2, buf2, &len2);

    lnr_cnt_merge_raw_bytes(ctx, buf1, len1, buf2, len2, NULL);
    esti = lnr_cnt_card(ctx);
    printf("actual:40000, estimated: %lu, error: %.2f%%\n",
           (long unsigned int)esti, fabs((double)(esti - 40000) / 40000 * 100));

    lnr_cnt_fini(tbm2);
    lnr_cnt_fini(tbm1);
    lnr_cnt_fini(ctx);
}

/**
 * Tests serialize, unserialize and merge.
 *
 * <ol>
 * <li>Current context contains 1 to 20000</li>
 * <li>Tbm1 that contains 10000 to 30000 be serialized as buf1</li>
 * <li>Tbm2 that contains 20000 to 40000 be serialized as buf2</li>
 * <li>Merges buf1 and buf2 into current context</li>
 * </ol>
 * */
TEST(LinearCounting, Merge)
{
    int64_t i, esti;
    lnr_cnt_ctx_t *ctx = lnr_cnt_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *tbm1 = lnr_cnt_init(NULL, 16, CCARD_HASH_MURMUR);
    lnr_cnt_ctx_t *tbm2 = lnr_cnt_init(NULL, 16, CCARD_HASH_MURMUR);
    int32_t m = 1<<16;
    uint8_t buf1[m + 3], buf2[m + 3];
    uint32_t len1 = m + 3, len2 = m + 3;

    for (i = 1; i <= 20000L; i++) {
        lnr_cnt_offer(ctx, &i, sizeof(uint64_t));
    }
    for (i = 10000L; i <= 30000L; i++) {
        lnr_cnt_offer(tbm1, &i, sizeof(uint64_t));
    }
    lnr_cnt_get_bytes(tbm1, buf1, &len1);
    for (i = 20000L; i <= 40000L; i++) {
        lnr_cnt_offer(tbm2, &i, sizeof(uint64_t));
    }
    lnr_cnt_get_bytes(tbm2, buf2, &len2);

    lnr_cnt_merge_bytes(ctx, buf1, len1, buf2, len2, NULL);
    esti = lnr_cnt_card(ctx);
    printf("actual:40000, estimated: %lu, error: %.2f%%\n",
           (long unsigned int)esti, fabs((double)(esti - 40000) / 40000 * 100));

    lnr_cnt_fini(tbm2);
    lnr_cnt_fini(tbm1);
    lnr_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

