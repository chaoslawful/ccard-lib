#include <math.h>
#include "ccard_common.h"
#include "adaptive_counting.h"
#include "gtest/gtest.h"

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use AdaptiveCounting and LogLogCounting algorithm
 * </p>
 * */
TEST(AdaptiveCounting, RawCounting)
{
    int64_t i, esti;
    adp_cnt_ctx_t *ctx1 = adp_cnt_raw_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *ctx2 = adp_cnt_raw_init(NULL, 16, CCARD_HASH_MURMUR);

    printf("Adaptive Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_reset(ctx1);

    printf("Loglog Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card_loglog(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    printf("Adaptive Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_reset(ctx2);

    printf("Loglog Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card_loglog(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_fini(ctx2);
    adp_cnt_fini(ctx1);
}

/**
 * Tests estimate.
 *
 * <p>
 * From 1 to 500000 distinct elements print estimated value every 50000.
 * Use AdaptiveCounting and LogLogCounting algorithm
 * </p>
 * */
TEST(AdaptiveCounting, Counting)
{
    int64_t i, esti;
    adp_cnt_ctx_t *ctx1 = adp_cnt_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *ctx2 = adp_cnt_init(NULL, 16, CCARD_HASH_MURMUR);

    printf("Adaptive Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_reset(ctx1);

    printf("Loglog Counting with Lookup3hash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx1, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card_loglog(ctx1);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    printf("Adaptive Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_reset(ctx2);

    printf("Loglog Counting with Murmurhash:\n");
    for (i = 1; i <= 500000L; i++) {
        adp_cnt_offer(ctx2, &i, sizeof(int64_t));

        if (i % 50000 == 0) {
            esti = adp_cnt_card_loglog(ctx2);
            printf("actual: %lu, estimated: %lu, error: %.2f%%\n",
                   (long unsigned int)i, (long unsigned int)esti, fabs((double)(esti - i) / i * 100));
        }
    }
    printf("\n");

    adp_cnt_fini(ctx2);
    adp_cnt_fini(ctx1);
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
TEST(AdaptiveCounting, RawMerge)
{
    int64_t i, esti;
    adp_cnt_ctx_t *ctx = adp_cnt_raw_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *tbm1 = adp_cnt_raw_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *tbm2 = adp_cnt_raw_init(NULL, 16, CCARD_HASH_LOOKUP3);
    int32_t m = pow(2, 16);
    uint8_t buf1[m + 3], buf2[m + 3];
    uint32_t len1 = m + 3, len2 = m + 3;

    for (i = 1; i <= 20000L; i++) {
        adp_cnt_offer(ctx, &i, sizeof(uint64_t));
    }
    for (i = 10000L; i <= 30000L; i++) {
        adp_cnt_offer(tbm1, &i, sizeof(uint64_t));
    }
    adp_cnt_get_raw_bytes(tbm1, buf1, &len1);
    for (i = 20000L; i <= 40000L; i++) {
        adp_cnt_offer(tbm2, &i, sizeof(uint64_t));
    }
    adp_cnt_get_raw_bytes(tbm2, buf2, &len2);

    adp_cnt_merge_raw_bytes(ctx, buf1, len1, buf2, len2, NULL);
    esti = adp_cnt_card(ctx);
    printf("actual:40000, estimated: %lu, error: %.2f%%\n",
           (long unsigned int)esti, fabs((double)(esti - 40000) / 40000 * 100));

    adp_cnt_fini(tbm2);
    adp_cnt_fini(tbm1);
    adp_cnt_fini(ctx);
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
TEST(AdaptiveCounting, Merge)
{
    int64_t i, esti;
    adp_cnt_ctx_t *ctx = adp_cnt_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *tbm1 = adp_cnt_init(NULL, 16, CCARD_HASH_LOOKUP3);
    adp_cnt_ctx_t *tbm2 = adp_cnt_init(NULL, 16, CCARD_HASH_LOOKUP3);
    int32_t m = pow(2, 16);
    uint8_t buf1[m + 3], buf2[m + 3];
    uint32_t len1 = m + 3, len2 = m + 3;

    for (i = 1; i <= 20000L; i++) {
        adp_cnt_offer(ctx, &i, sizeof(uint64_t));
    }
    for (i = 10000L; i <= 30000L; i++) {
        adp_cnt_offer(tbm1, &i, sizeof(uint64_t));
    }
    adp_cnt_get_bytes(tbm1, buf1, &len1);
    for (i = 20000L; i <= 40000L; i++) {
        adp_cnt_offer(tbm2, &i, sizeof(uint64_t));
    }
    adp_cnt_get_bytes(tbm2, buf2, &len2);

    adp_cnt_merge_bytes(ctx, buf1, len1, buf2, len2, NULL);
    esti = adp_cnt_card(ctx);
    printf("actual:40000, estimated: %lu, error: %.2f%%\n",
           (long unsigned int)esti, fabs((double)(esti - 40000) / 40000 * 100));

    adp_cnt_fini(tbm2);
    adp_cnt_fini(tbm1);
    adp_cnt_fini(ctx);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

