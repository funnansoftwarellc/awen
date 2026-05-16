#include <gtest/gtest.h>

import awen.raylib.color;

TEST(Color, ToRaylibColor)
{
    const awen::raylib::Color color{.r = 255, .g = 127, .b = 0, .a = 255};
    const auto raylibColor = awen::raylib::ToRaylibColor(color);

    EXPECT_EQ(raylibColor.r, color.r);
    EXPECT_EQ(raylibColor.g, color.g);
    EXPECT_EQ(raylibColor.b, color.b);
    EXPECT_EQ(raylibColor.a, color.a);
}