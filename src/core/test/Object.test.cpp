#include <gtest/gtest.h>

import awen.core.object;

TEST(Object, Name)
{
    awen::core::Object object;
    object.setName("Test Object");
    EXPECT_EQ(object.getName(), "Test Object");
}

TEST(Object, ParentChild)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child = std::make_unique<awen::core::Object>();

    parent->addChild(std::move(child));
    ASSERT_EQ(std::size(parent->getChildren()), 1);
    EXPECT_EQ(parent->getChildren()[0]->getParent(), parent.get());

    auto removed_child = parent->getChildren()[0]->remove();
    EXPECT_EQ(removed_child->getParent(), nullptr);
    EXPECT_EQ(std::size(parent->getChildren()), 0);
}

TEST(Object, RemoveWithoutParent)
{
    auto object = std::make_unique<awen::core::Object>();
    auto removed_object = object->remove();
    EXPECT_EQ(removed_object, nullptr);
}

TEST(Object, RemoveNonExistentChild)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child = std::make_unique<awen::core::Object>();

    parent->addChild(std::move(child));
    ASSERT_EQ(std::size(parent->getChildren()), 1);

    auto removed_child = parent->getChildren()[0]->remove();
    EXPECT_EQ(removed_child->getParent(), nullptr);
    EXPECT_EQ(std::size(parent->getChildren()), 0);

    // Attempt to remove the same child again
    auto removed_child_again = removed_child->remove();
    EXPECT_EQ(removed_child_again, nullptr);
}

TEST(Object, MultipleChildren)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child1 = std::make_unique<awen::core::Object>();
    auto child2 = std::make_unique<awen::core::Object>();

    parent->addChild(std::move(child1));
    parent->addChild(std::move(child2));

    ASSERT_EQ(std::size(parent->getChildren()), 2);
    EXPECT_EQ(parent->getChildren()[0]->getParent(), parent.get());
    EXPECT_EQ(parent->getChildren()[1]->getParent(), parent.get());

    auto removed_child1 = parent->getChildren()[0]->remove();
    EXPECT_EQ(removed_child1->getParent(), nullptr);
    EXPECT_EQ(std::size(parent->getChildren()), 1);

    auto removed_child2 = parent->getChildren()[0]->remove();
    EXPECT_EQ(removed_child2->getParent(), nullptr);
    EXPECT_EQ(std::size(parent->getChildren()), 0);
}

TEST(Object, OnDestroyedSignal)
{
    auto object = std::make_unique<awen::core::Object>();
    auto destroyed_count = int{};

    object->onDestroyed().connect([&] { ++destroyed_count; });

    EXPECT_EQ(destroyed_count, 0);
    object.reset();
    EXPECT_EQ(destroyed_count, 1);
}

TEST(Object, OnDestroyedSignalWithChildren)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child = std::make_unique<awen::core::Object>();
    parent->addChild(std::move(child));

    auto parent_destroyed_count = int{};
    auto child_destroyed_count = int{};

    parent->onDestroyed().connect([&] { ++parent_destroyed_count; });
    parent->getChildren()[0]->onDestroyed().connect([&] { ++child_destroyed_count; });

    EXPECT_EQ(parent_destroyed_count, 0);
    EXPECT_EQ(child_destroyed_count, 0);

    parent.reset();

    EXPECT_EQ(parent_destroyed_count, 1);
    EXPECT_EQ(child_destroyed_count, 1);
}

TEST(Object, OnStartupSignal)
{
    auto object = std::make_unique<awen::core::Object>();
    auto startup_count = int{};

    object->onStartup().connect([&] { ++startup_count; });

    EXPECT_EQ(startup_count, 0);
    object->startup();
    EXPECT_EQ(startup_count, 1);
}

TEST(Object, OnStartupSignalWithChildren)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child = std::make_unique<awen::core::Object>();
    parent->addChild(std::move(child));

    auto parent_startup_count = int{};
    auto child_startup_count = int{};

    parent->onStartup().connect([&] { ++parent_startup_count; });
    parent->getChildren()[0]->onStartup().connect([&] { ++child_startup_count; });

    EXPECT_EQ(parent_startup_count, 0);
    EXPECT_EQ(child_startup_count, 0);

    parent->startup();

    EXPECT_EQ(parent_startup_count, 1);
    EXPECT_EQ(child_startup_count, 1);
}

TEST(Object, StartupAfterAddingChild)
{
    auto parent = std::make_unique<awen::core::Object>();
    auto child = std::make_unique<awen::core::Object>();

    auto child_startup_count = int{};
    child->onStartup().connect([&] { ++child_startup_count; });

    parent->startup();
    EXPECT_EQ(child_startup_count, 0);

    parent->addChild(std::move(child));
    EXPECT_EQ(child_startup_count, 1);
}
