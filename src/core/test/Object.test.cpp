#include <gtest/gtest.h>

import awen.core.object;

using awen::core::Object;

TEST(Object, OnDestroyedSignal)
{
    Object obj;
    bool destroyed = false;
    obj.onDestroyed.connect([&]() { destroyed = true; });
    EXPECT_FALSE(destroyed);
}