#include <gtest/gtest.h>

import awen.core.object;

using awen::core::Object;

TEST(Object, OnDestroyedSignal)
{
    Object obj;
    bool destroyed = false;
    std::ignore = obj.onDestroyed.connect([&]() { destroyed = true; });
    EXPECT_FALSE(destroyed);
}

TEST(Object, AddChild)
{
    Object parent;
    auto child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);
}

TEST(Object, AddChildWithArgs)
{
    struct DerivedObject : public Object
    {
        DerivedObject(int value) : value(value)
        {
        }

        int value;
    };

    Object parent;
    auto child = parent.addChild<DerivedObject>(42);
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->value, 42);
}