#include "register_set.h"
#include "gtest/gtest.h"

/**
 * Tests set and get value of register set.
 *
 * <p>
 * </p>
 * */
TEST(RegisterSetTest, GetAndSet)
{
    reg_set_t *rs = rs_init(1024, NULL, 0);
    uint32_t value;
    int i;

    for (i = 0; i < 1023; i++) {
        rs_set(rs, i, i % 32);
    }

    for (i = 0; i < 1023; i++) {
        if (i % 100 == 0) {
            rs_get(rs, i, &value);
            EXPECT_EQ((uint32_t)(i % 32), value);
        }
    }

    rs_fini(rs);
}

// vi:ft=c ts=4 sw=4 fdm=marker et

