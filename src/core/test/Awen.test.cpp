#include <gtest/gtest.h>
#include <limits>

import awen;

TEST(Awen, value)
{
    awn::Awen awen;
    awen.set_value(std::numeric_limits<int>::max());
    EXPECT_EQ(awen.get_value(), std::numeric_limits<int>::max());
}