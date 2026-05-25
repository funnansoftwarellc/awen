#include <awen/test/Test.hpp>

import awen.core.object;

using awen::core::Object;

UNIT_TEST(Object, OnDestroyedSignal)
{
    Object obj;
    bool destroyed = false;
    std::ignore = obj.onDestroyed([&]() { destroyed = true; });
    EXPECT_FALSE(destroyed);
}

UNIT_TEST(Object, AddChild)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);
}

UNIT_TEST(Object, AddChildWithArgs)
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

UNIT_TEST(Object, AddChildEmitsChildAddAfterParentSet)
{
    Object parent;
    bool signalCalled = false;
    bool signalSawParentSet = false;
    Object* signalChild{};

    std::ignore = parent.onChildAdd(
        [&](Object& child)
        {
            signalCalled = true;
            signalSawParentSet = child.getParent() == &parent;
            signalChild = &child;
        });

    auto* child = parent.addChild<Object>();

    EXPECT_NE(child, nullptr);
    EXPECT_TRUE(signalCalled);
    EXPECT_TRUE(signalSawParentSet);
    EXPECT_EQ(signalChild, child);
}

UNIT_TEST(Object, AddChildNullDoesNotEmitChildAdd)
{
    Object parent;
    bool signalCalled = false;

    std::ignore = parent.onChildAdd([&](Object&) { signalCalled = true; });

    auto* child = parent.addChild(std::unique_ptr<Object>{});

    EXPECT_EQ(child, nullptr);
    EXPECT_FALSE(signalCalled);
}

UNIT_TEST(Object, RemoveChild)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);

    auto removedChild = child->remove();
    EXPECT_NE(removedChild, nullptr);
    EXPECT_EQ(removedChild.get(), child);
    EXPECT_EQ(child->getParent(), nullptr);
}

UNIT_TEST(Object, RemoveChildEmitsChildRemoveAfterParentCleared)
{
    Object parent;
    auto* child = parent.addChild<Object>();
    bool signalCalled = false;
    bool signalSawParentCleared = false;
    Object* signalChild{};

    std::ignore = parent.onChildRemove(
        [&](Object& removedChild)
        {
            signalCalled = true;
            signalSawParentCleared = removedChild.getParent() == nullptr;
            signalChild = &removedChild;
        });

    auto removedChild = child->remove();

    EXPECT_NE(removedChild, nullptr);
    EXPECT_EQ(removedChild.get(), child);
    EXPECT_TRUE(signalCalled);
    EXPECT_TRUE(signalSawParentCleared);
    EXPECT_EQ(signalChild, child);
    EXPECT_TRUE(std::empty(parent.getChildren()));
}

UNIT_TEST(Object, GetParent)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->getParent(), &parent);
}

UNIT_TEST(Object, GetParentWithType)
{
    struct DerivedObject : public Object
    {
    };

    DerivedObject parent;
    auto* child = parent.addChild<Object>();
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->getParent<DerivedObject>(), &parent);
}

UNIT_TEST(Object, GetParentWithTypeNotFound)
{
    struct DerivedObject : public Object
    {
    };

    Object parent;
    auto* child = parent.addChild<Object>();
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->getParent<DerivedObject>(), nullptr);
}

UNIT_TEST(Object, GetParentWithTypeNoParent)
{
    struct DerivedObject : public Object
    {
    };

    Object obj;
    EXPECT_EQ(obj.getParent<DerivedObject>(), nullptr);
}

UNIT_TEST(Object, GetParentWithTypeCache)
{
    struct DerivedObject : public Object
    {
    };

    Object parent;
    auto* child = parent.addChild(std::make_unique<DerivedObject>());
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->getParent<DerivedObject>(), nullptr);

    auto* derivedChild = child->addChild(std::make_unique<DerivedObject>());
    EXPECT_NE(derivedChild, nullptr);
    EXPECT_EQ(derivedChild->getParent<DerivedObject>(), child);
}

UNIT_TEST(Object, GetParentWithTypeCacheClearedWhenSubtreeMoves)
{
    struct FirstParent : public Object
    {
    };

    struct SecondParent : public Object
    {
    };

    FirstParent firstParent;
    SecondParent secondParent;
    auto* child = firstParent.addChild<Object>();
    auto* grandchild = child->addChild<Object>();

    EXPECT_EQ(grandchild->getParent<FirstParent>(), &firstParent);

    auto removedChild = child->remove();
    auto* readdedChild = secondParent.addChild(std::move(removedChild));

    EXPECT_EQ(readdedChild, child);
    EXPECT_EQ(grandchild->getParent<FirstParent>(), nullptr);
    EXPECT_EQ(grandchild->getParent<SecondParent>(), &secondParent);
}

UNIT_TEST(Object, GetChildrenEmpty)
{
    Object parent;
    const auto& children = parent.getChildren();
    EXPECT_TRUE(children.empty());
}

UNIT_TEST(Object, GetChildren)
{
    Object parent;
    auto* child1 = parent.addChild(std::make_unique<Object>());
    auto* child2 = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    const auto& children = parent.getChildren();
    EXPECT_EQ(children.size(), 2);
    EXPECT_EQ(children.at(0), child1);
    EXPECT_EQ(children.at(1), child2);
}

UNIT_TEST(Object, GetChildrenRecursive)
{
    Object parent;
    auto* child1 = parent.addChild(std::make_unique<Object>());
    auto* child2 = child1->addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    const auto& children = parent.getChildren(Object::FindOption::Recursive);
    EXPECT_EQ(children.size(), 2);
    EXPECT_EQ(children.at(0), child1);
    EXPECT_EQ(children.at(1), child2);
}

UNIT_TEST(Object, GetChildrenWithPredicate)
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

UNIT_TEST(Object, GetChildrenWithPredicateRecursive)
{
    Object parent;
    auto* child1 = parent.addChild(std::make_unique<Object>());
    auto* child2 = child1->addChild(std::make_unique<Object>());
    EXPECT_NE(child1, nullptr);
    EXPECT_NE(child2, nullptr);

    auto children = parent.getChildren([](const Object*) { return true; }, Object::FindOption::Recursive);
    EXPECT_EQ(children.size(), 2);
    EXPECT_EQ(children.at(0), child1);
    EXPECT_EQ(children.at(1), child2);
}

UNIT_TEST(Object, UpdatePreSignal)
{
    Object obj;
    bool called = false;
    std::ignore = obj.onUpdatePre([&]() { called = true; });

    obj.updatePre();
    EXPECT_TRUE(called);
}

UNIT_TEST(Object, UpdateSignal)
{
    Object obj;
    std::optional<float> expected;
    std::ignore = obj.onUpdate([&](auto x) { expected = x.count(); });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    obj.update(duration);
    EXPECT_TRUE(expected.has_value());
}

UNIT_TEST(Object, UpdateFixedSignal)
{
    Object obj;
    std::optional<float> expected;
    std::ignore = obj.onUpdateFixed([&](auto x) { expected = x.count(); });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    obj.updateFixed(duration);
    EXPECT_TRUE(expected.has_value());
}

UNIT_TEST(Object, UpdatePostSignal)
{
    Object obj;
    bool called = false;
    std::ignore = obj.onUpdatePost([&]() { called = true; });

    obj.updatePost();
    EXPECT_TRUE(called);
}

UNIT_TEST(Object, UpdateRecursive)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);

    bool parentCalled = false;
    bool childCalled = false;
    std::ignore = parent.onUpdate([&](auto) { parentCalled = true; });
    std::ignore = child->onUpdate([&](auto) { childCalled = true; });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    parent.update(duration);
    EXPECT_TRUE(parentCalled);
    EXPECT_TRUE(childCalled);
}

UNIT_TEST(Object, UpdateFixedRecursive)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);

    bool parentCalled = false;
    bool childCalled = false;
    std::ignore = parent.onUpdateFixed([&](auto) { parentCalled = true; });
    std::ignore = child->onUpdateFixed([&](auto) { childCalled = true; });

    constexpr auto duration = std::chrono::duration<float>(0.1F);
    parent.updateFixed(duration);
    EXPECT_TRUE(parentCalled);
    EXPECT_TRUE(childCalled);
}

UNIT_TEST(Object, StartupSignal)
{
    Object obj;
    bool called = false;
    std::ignore = obj.onStartup([&]() { called = true; });

    obj.startup();
    EXPECT_TRUE(called);
}

UNIT_TEST(Object, StartupRecursive)
{
    Object parent;
    auto* child = parent.addChild(std::make_unique<Object>());
    EXPECT_NE(child, nullptr);

    bool parentCalled = false;
    bool childCalled = false;
    std::ignore = parent.onStartup([&]() { parentCalled = true; });
    std::ignore = child->onStartup([&]() { childCalled = true; });

    parent.startup();
    EXPECT_TRUE(parentCalled);
    EXPECT_TRUE(childCalled);
}

UNIT_TEST(Object, StartupMultipleCalls)
{
    Object obj;
    int callCount = 0;
    std::ignore = obj.onStartup([&]() { ++callCount; });

    obj.startup();
    obj.startup();
    EXPECT_EQ(callCount, 1);
}