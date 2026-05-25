#include <awen/test/Test.hpp>

import awen.core.object;
import awen.raylib.node;
import awen.raylib.rectangle;
import awen.raylib.window;
import awen.raylib.color;
import awen.raylib.events;

// NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

GRAPHICS_TEST(Node, addNode)
{
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    auto* child = nodePtr->addNode(std::move(node));

    EXPECT_EQ(child, nodePtr);
}

GRAPHICS_TEST(Node, addNodeTemplate)
{
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    auto* child = nodePtr->addNode<awen::raylib::Node>();

    EXPECT_NE(child, nodePtr);
}

GRAPHICS_TEST(Node, getNodes)
{
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    auto* child1 = nodePtr->addNode<awen::raylib::Node>();
    auto* child2 = nodePtr->addNode<awen::raylib::Node>();

    auto nodes = nodePtr->getNodes();

    EXPECT_EQ(nodes.size(), 2);
    EXPECT_EQ(nodes.at(0), child1);
    EXPECT_EQ(nodes.at(1), child2);
}

GRAPHICS_TEST(Node, renderPre)
{
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    bool called = false;

    nodePtr->onRenderPre([&called] { called = true; });

    nodePtr->renderPre();

    EXPECT_TRUE(called);
}

GRAPHICS_TEST(Node, render)
{
    auto window = std::make_unique<awen::raylib::Window>();
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    bool called = false;

    nodePtr->onRender([&called] { called = true; });

    nodePtr->render();

    EXPECT_TRUE(called);
}

GRAPHICS_TEST(Node, renderPost)
{
    auto node = std::make_unique<awen::raylib::Node>();
    auto* nodePtr = node.get();

    bool called = false;

    nodePtr->onRenderPost([&called] { called = true; });

    nodePtr->renderPost();

    EXPECT_TRUE(called);
}

// A mouse click whose world position falls inside a nested child rectangle
// must be handled by the child, not the parent.
UNIT_TEST(Node, NestedRectangleMouseEventRoutedToChild)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();

    auto* rectOne = rootNode->addNode<awen::raylib::Rectangle>();
    rectOne->setPosition({.x = 100.0F, .y = 100.0F});
    rectOne->setWidth(200.0F);
    rectOne->setHeight(200.0F);
    rectOne->setColor(awen::raylib::colors::Red);

    auto* rectOneChild = rectOne->addNode<awen::raylib::Rectangle>();
    rectOneChild->setPosition({.x = 10.0F, .y = 10.0F});
    rectOneChild->setWidth(50.0F);
    rectOneChild->setHeight(50.0F);
    rectOneChild->setColor(awen::raylib::colors::Blue);

    // World position (120, 120) is inside both rectangles.
    // The child's local position is (10, 10) and its visual screen position is (110, 110)..
    // After coordinate transform, the child receives local coords (10, 10),
    // which falls within its 50x50 bounds, so it should consume the event.
    awen::raylib::Event e = awen::raylib::EventMouse{
        .type = awen::raylib::EventMouse::Type::ButtonPressed,
        .button = awen::raylib::EventMouse::Button::Left,
        .x = 120.0F,
        .y = 120.0F,
    };

    rootNode->events(e);

    EXPECT_EQ(rectOneChild->getColor(), awen::raylib::colors::Green);
    EXPECT_EQ(rectOne->getColor(), awen::raylib::colors::Red);
}

// A click inside the parent but outside the child should be handled by the parent.
UNIT_TEST(Node, NestedRectangleMouseEventRoutedToParent)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();

    auto* rectOne = rootNode->addNode<awen::raylib::Rectangle>();
    rectOne->setPosition({.x = 100.0F, .y = 100.0F});
    rectOne->setWidth(200.0F);
    rectOne->setHeight(200.0F);
    rectOne->setColor(awen::raylib::colors::Red);

    auto* rectOneChild = rectOne->addNode<awen::raylib::Rectangle>();
    rectOneChild->setPosition({.x = 10.0F, .y = 10.0F});
    rectOneChild->setWidth(50.0F);
    rectOneChild->setHeight(50.0F);
    rectOneChild->setColor(awen::raylib::colors::Blue);

    // World position (200, 200) is inside rectOne but outside rectOneChild.
    // rectOneChild's screen bounds are (110, 110)..(160, 160).
    awen::raylib::Event e = awen::raylib::EventMouse{
        .type = awen::raylib::EventMouse::Type::ButtonPressed,
        .button = awen::raylib::EventMouse::Button::Left,
        .x = 200.0F,
        .y = 200.0F,
    };

    rootNode->events(e);

    EXPECT_EQ(rectOneChild->getColor(), awen::raylib::colors::Blue);
    EXPECT_EQ(rectOne->getColor(), awen::raylib::colors::Green);
}

UNIT_TEST(Node, RemoveUpdatesParentNodeList)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();

    auto* childNode = rootNode->addNode<awen::raylib::Node>();

    auto removedNode = childNode->remove();

    EXPECT_NE(removedNode, nullptr);
    EXPECT_EQ(removedNode.get(), childNode);

    const auto nodes = rootNode->getNodes();

    EXPECT_TRUE(std::empty(nodes));
}

UNIT_TEST(Node, AddNodeThroughObjectUpdatesNodeList)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    auto childNode = std::make_unique<awen::raylib::Node>();
    auto* childNodePtr = childNode.get();
    auto* objectParent = static_cast<awen::core::Object*>(rootNode.get());

    auto* childObject = objectParent->addChild(std::move(childNode));
    const auto nodes = rootNode->getNodes();

    EXPECT_EQ(childObject, childNodePtr);
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0), childNodePtr);
}

UNIT_TEST(Node, ChildAddSignalRunsAfterNodeListUpdates)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    auto childNode = std::make_unique<awen::raylib::Node>();
    auto* childNodePtr = childNode.get();
    auto* objectParent = static_cast<awen::core::Object*>(rootNode.get());
    bool signalSawNode = false;

    std::ignore = rootNode->onChildAdd(
        [&](awen::core::Object&)
        {
            const auto nodes = rootNode->getNodes();
            signalSawNode = std::size(nodes) == 1 && nodes.at(0) == childNodePtr;
        });

    std::ignore = objectParent->addChild(std::move(childNode));

    EXPECT_TRUE(signalSawNode);
}

UNIT_TEST(Node, RemoveNodeThroughObjectUpdatesNodeList)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    auto* childNode = rootNode->addNode<awen::raylib::Node>();
    auto* childObject = static_cast<awen::core::Object*>(childNode);

    auto removedChild = childObject->remove();
    const auto nodes = rootNode->getNodes();

    EXPECT_NE(removedChild, nullptr);
    EXPECT_EQ(removedChild.get(), childNode);
    EXPECT_TRUE(std::empty(nodes));
}

UNIT_TEST(Node, ChildRemoveSignalRunsAfterNodeListUpdates)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    auto* childNode = rootNode->addNode<awen::raylib::Node>();
    bool signalSawNodeRemoved = false;

    std::ignore = rootNode->onChildRemove([&](awen::core::Object&) { signalSawNodeRemoved = std::empty(rootNode->getNodes()); });

    std::ignore = childNode->remove();

    EXPECT_TRUE(signalSawNodeRemoved);
}

UNIT_TEST(Node, AddPlainObjectThroughObjectDoesNotUpdateNodeList)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    auto* objectParent = static_cast<awen::core::Object*>(rootNode.get());

    auto* childObject = objectParent->addChild(std::make_unique<awen::core::Object>());
    const auto children = rootNode->getChildren();
    const auto nodes = rootNode->getNodes();

    EXPECT_NE(childObject, nullptr);
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children.at(0), childObject);
    EXPECT_TRUE(std::empty(nodes));
}

UNIT_TEST(Node, AddNodeThroughObjectUpdatesTransformParent)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();
    rootNode->setPosition({.x = 100.0F, .y = 50.0F});

    auto childNode = std::make_unique<awen::raylib::Node>();
    auto* childNodePtr = childNode.get();
    childNodePtr->setPosition({.x = 10.0F, .y = 5.0F});

    auto* objectParent = static_cast<awen::core::Object*>(rootNode.get());
    std::ignore = objectParent->addChild(std::move(childNode));

    const auto worldPosition = childNodePtr->mapToWorld({.x = 0.0F, .y = 0.0F});

    EXPECT_FLOAT_EQ(worldPosition.x, 110.0F);
    EXPECT_FLOAT_EQ(worldPosition.y, 55.0F);
}

UNIT_TEST(Node, OverlappingSiblingMouseEventRoutedToLastDrawnNode)
{
    auto rootNode = std::make_unique<awen::raylib::Node>();

    auto* backRectangle = rootNode->addNode<awen::raylib::Rectangle>();
    backRectangle->setWidth(100.0F);
    backRectangle->setHeight(100.0F);
    backRectangle->setColor(awen::raylib::colors::Red);

    auto* frontRectangle = rootNode->addNode<awen::raylib::Rectangle>();
    frontRectangle->setWidth(100.0F);
    frontRectangle->setHeight(100.0F);
    frontRectangle->setColor(awen::raylib::colors::Blue);

    awen::raylib::Event event = awen::raylib::EventMouse{
        .type = awen::raylib::EventMouse::Type::ButtonPressed,
        .button = awen::raylib::EventMouse::Button::Left,
        .x = 50.0F,
        .y = 50.0F,
    };

    rootNode->events(event);

    EXPECT_EQ(backRectangle->getColor(), awen::raylib::colors::Red);
    EXPECT_EQ(frontRectangle->getColor(), awen::raylib::colors::Green);
}

// NOLINTEND (cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
