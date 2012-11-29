#include "murmurhash.h"
#include "gtest/gtest.h"

/**
 * Tests Murmurhash with byte[] buffer.
 *
 * <p>
 * When using java stream-lib the hash code of "hello world" is 1964480955
 * </p>
 * */
TEST(MurmurhashTest, Buffer) {
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
TEST(MurmurhashTest, Long) {
    uint64_t longint = 123456;

    EXPECT_EQ(148129653lu, murmurhash_long(longint));
}

// vi:ft=c ts=4 sw=4 fdm=marker et
