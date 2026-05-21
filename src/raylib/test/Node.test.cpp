#include <awen/test/Test.hpp>

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

// NOLINTEND (cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
