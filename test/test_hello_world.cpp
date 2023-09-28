#include <gtest/gtest.h>

TEST(StringsTest, StringAssertions)
{
    EXPECT_STRNE("hello", "world");
}