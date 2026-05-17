#include <awen/test/Test.hpp>

import awen.raylib.node;
import awen.raylib.window;

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