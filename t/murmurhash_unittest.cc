#include "murmurhash.h"
#include "gtest/gtest.h"

/**
 * Tests Murmurhash with byte[] buffer.
 *
 * <p>
 * When using java stream-lib the hash code of "hello world" is 1964480955
 * </p>
 * */
TEST(MurmurhashTest, Buffer)
{
    char *s = (char *)"hello world";

    EXPECT_EQ(1964480955lu, murmurhash((void *)s, strlen(s), -1));
}

/**
 * Tests Murmurhash with 64bit long integer.
 *
 * <p>
 * When using java stream-lib the hash code of 123456 is 148129653
 * </p>
 * */
TEST(MurmurhashTest, Long)
{
    uint64_t longint = 123456;

    EXPECT_EQ(148129653lu, murmurhash_long(longint));
}

/**
 * Tests Murmurhash64 with byte[] buffer.
 *
 * <p>
 * When using java stream-lib the hash code (with sign) of "hello world" is -779442749388864765
 * </p>
 * */
TEST(Murmurhash64Test, Buffer)
{
    char *s = (char *)"hello world";

    EXPECT_EQ(-779442749388864765l, (int64_t)murmurhash64_no_seed((void *)s, strlen(s)));
}

// vi:ft=c ts=4 sw=4 fdm=marker et
