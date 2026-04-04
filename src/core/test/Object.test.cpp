#include <gtest/gtest.h>

import awen.core.object;

TEST(Object, Name)
{
    awn::core::Object object;
    object.set_name("Test Object");
    EXPECT_EQ(object.get_name(), "Test Object");
}

TEST(Object, ParentChild)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child = std::make_unique<awn::core::Object>();

    parent->add_child(std::move(child));
    ASSERT_EQ(std::size(parent->get_children()), 1);
    EXPECT_EQ(parent->get_children()[0]->get_parent(), parent.get());

    auto removed_child = parent->get_children()[0]->remove();
    EXPECT_EQ(removed_child->get_parent(), nullptr);
    EXPECT_EQ(std::size(parent->get_children()), 0);
}

TEST(Object, RemoveWithoutParent)
{
    auto object = std::make_unique<awn::core::Object>();
    auto removed_object = object->remove();
    EXPECT_EQ(removed_object, nullptr);
}

TEST(Object, RemoveNonExistentChild)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child = std::make_unique<awn::core::Object>();

    parent->add_child(std::move(child));
    ASSERT_EQ(std::size(parent->get_children()), 1);

    auto removed_child = parent->get_children()[0]->remove();
    EXPECT_EQ(removed_child->get_parent(), nullptr);
    EXPECT_EQ(std::size(parent->get_children()), 0);

    // Attempt to remove the same child again
    auto removed_child_again = removed_child->remove();
    EXPECT_EQ(removed_child_again, nullptr);
}

TEST(Object, MultipleChildren)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child1 = std::make_unique<awn::core::Object>();
    auto child2 = std::make_unique<awn::core::Object>();

    parent->add_child(std::move(child1));
    parent->add_child(std::move(child2));

    ASSERT_EQ(std::size(parent->get_children()), 2);
    EXPECT_EQ(parent->get_children()[0]->get_parent(), parent.get());
    EXPECT_EQ(parent->get_children()[1]->get_parent(), parent.get());

    auto removed_child1 = parent->get_children()[0]->remove();
    EXPECT_EQ(removed_child1->get_parent(), nullptr);
    EXPECT_EQ(std::size(parent->get_children()), 1);

    auto removed_child2 = parent->get_children()[0]->remove();
    EXPECT_EQ(removed_child2->get_parent(), nullptr);
    EXPECT_EQ(std::size(parent->get_children()), 0);
}

TEST(Object, OnDestroyedSignal)
{
    auto object = std::make_unique<awn::core::Object>();
    auto destroyed_count = int{};

    object->on_destroyed.connect([&] { ++destroyed_count; });

    EXPECT_EQ(destroyed_count, 0);
    object.reset();
    EXPECT_EQ(destroyed_count, 1);
}

TEST(Object, OnDestroyedSignalWithChildren)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child = std::make_unique<awn::core::Object>();
    parent->add_child(std::move(child));

    auto parent_destroyed_count = int{};
    auto child_destroyed_count = int{};

    parent->on_destroyed.connect([&] { ++parent_destroyed_count; });
    parent->get_children()[0]->on_destroyed.connect([&] { ++child_destroyed_count; });

    EXPECT_EQ(parent_destroyed_count, 0);
    EXPECT_EQ(child_destroyed_count, 0);

    parent.reset();

    EXPECT_EQ(parent_destroyed_count, 1);
    EXPECT_EQ(child_destroyed_count, 1);
}

TEST(Object, OnStartupSignal)
{
    auto object = std::make_unique<awn::core::Object>();
    auto startup_count = int{};

    object->on_startup.connect([&] { ++startup_count; });

    EXPECT_EQ(startup_count, 0);
    object->startup();
    EXPECT_EQ(startup_count, 1);
}

TEST(Object, OnStartupSignalWithChildren)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child = std::make_unique<awn::core::Object>();
    parent->add_child(std::move(child));

    auto parent_startup_count = int{};
    auto child_startup_count = int{};

    parent->on_startup.connect([&] { ++parent_startup_count; });
    parent->get_children()[0]->on_startup.connect([&] { ++child_startup_count; });

    EXPECT_EQ(parent_startup_count, 0);
    EXPECT_EQ(child_startup_count, 0);

    parent->startup();

    EXPECT_EQ(parent_startup_count, 1);
    EXPECT_EQ(child_startup_count, 1);
}

TEST(Object, StartupAfterAddingChild)
{
    auto parent = std::make_unique<awn::core::Object>();
    auto child = std::make_unique<awn::core::Object>();

    auto child_startup_count = int{};
    child->on_startup.connect([&] { ++child_startup_count; });

    parent->startup();
    EXPECT_EQ(child_startup_count, 0);

    parent->add_child(std::move(child));
    EXPECT_EQ(child_startup_count, 1);
}
