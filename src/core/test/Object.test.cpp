#include <gtest/gtest.h>

import awen.core.object;

using awen::core::Object;

TEST(Object, OnDestroyedSignal)
{
    Object obj;
    bool destroyed = false;
    std::ignore = obj.onDestroyed([&]() { destroyed = true; });
    EXPECT_FALSE(destroyed);
}

TEST(Object, AddChild)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
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
    constexpr auto expectedValue = 42;
    auto* child = parent.addChild<DerivedObject>(expectedValue);
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->value, expectedValue);
}

TEST(Object, RemoveChild)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);

    auto removedChild = child->remove();
    EXPECT_NE(removedChild, nullptr);
    EXPECT_EQ(removedChild.get(), child);
    EXPECT_EQ(child->getParent(), nullptr);
}

TEST(Object, GetParent)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->getParent(), &parent);
}

TEST(Object, GetChildren)
{
    Object parent;
    auto* child1 = parent.addChild(std::make_unique<Object>());
    auto* child2 = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    const auto& children = parent.getChildren();
    EXPECT_EQ(children.size(), 2);
    EXPECT_EQ(children.at(0).get(), child1);
    EXPECT_EQ(children.at(1).get(), child2);
}

TEST(Object, GetChildrenWithPredicate)
{
    Object parent;
    auto* child1 = parent.addChild(std::make_unique<Object>());
    auto* child2 = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    auto children = parent.getChildren([child1](const Object* child) { return child == child1; });
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children.at(0), child1);
}

TEST(Object, GetChildrenOfType)
{
    struct DerivedObject : public Object
    {
    };

    Object parent;
    auto* child1 = parent.addChild(std::make_unique<DerivedObject>());
    auto* child2 = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    auto children = parent.getChildren<DerivedObject>();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children.at(0), child1);
}

TEST(Object, UpdateSignal)
{
    Object obj;
    std::optional<float> expected;
    std::ignore = obj.onUpdate([&](auto x) { expected = x.count(); });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    obj.update(duration);
    EXPECT_TRUE(expected.has_value());
}

TEST(Object, UpdateFixedSignal)
{
    Object obj;
    std::optional<float> expected;
    std::ignore = obj.onUpdateFixed([&](auto x) { expected = x.count(); });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    obj.updateFixed(duration);
    EXPECT_TRUE(expected.has_value());
}

TEST(Object, RenderPreSignal)
{
    Object obj;
    bool rendered = false;
    std::ignore = obj.onRenderPre([&]() { rendered = true; });
    obj.renderPre();
    EXPECT_TRUE(rendered);
}

TEST(Object, RenderSignal)
{
    Object obj;
    bool rendered = false;
    std::ignore = obj.onRender([&]() { rendered = true; });
    obj.render();
    EXPECT_TRUE(rendered);
}

TEST(Object, RenderPostSignal)
{
    Object obj;
    bool rendered = false;
    std::ignore = obj.onRenderPost([&]() { rendered = true; });
    obj.renderPost();
    EXPECT_TRUE(rendered);
}