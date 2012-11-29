#include "lookup3hash.h"
#include "gtest/gtest.h"

/**
 * Tests Lookup3hash with byte[] buffer, the hash code is 64bit.
 *
 * <p>
 * When using java stream-lib the hash code of "hello world" is 4141157809988715033
 * </p>
 * */
TEST(Lookup3hashTest, StringHashToLong) {
    const char *s = "hello world";

    EXPECT_EQ(4141157809988715033lu, lookup3ycs64_2(s));
}

// vi:ft=c ts=4 sw=4 fdm=marker et

