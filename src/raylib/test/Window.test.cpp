#include <gtest/gtest.h>
#include <awen/test/Test.hpp>

import awen.raylib.node;
import awen.raylib.window;

GRAPHICS_TEST(Window, Constructor)
{
    const awen::raylib::Window window{};
    EXPECT_TRUE(true);
}

GRAPHICS_TEST(Window, UpdatePostRendersRootNodeCallbacks)
{
    awen::raylib::Window window{};

    auto* rootNode = window.getRootNode();
    bool renderPreCalled = false;
    bool renderCalled = false;
    bool renderPostCalled = false;

    rootNode->onRenderPre([&renderPreCalled] { renderPreCalled = true; });
    rootNode->onRender([&renderCalled] { renderCalled = true; });
    rootNode->onRenderPost([&renderPostCalled] { renderPostCalled = true; });

    window.updatePost();

    EXPECT_TRUE(renderPreCalled);
    EXPECT_TRUE(renderCalled);
    EXPECT_TRUE(renderPostCalled);
}

GRAPHICS_TEST(Window, SetRootNodeReplacesDefaultRootInLifecycle)
{
    awen::raylib::Window window{};

    auto* oldRootNode = window.getRootNode();
    bool oldRootUpdated = false;

    oldRootNode->onUpdatePre([&oldRootUpdated] { oldRootUpdated = true; });

    auto replacementRootNode = std::make_unique<awen::raylib::Node>();
    auto* newRootNode = replacementRootNode.get();
    bool newRootUpdated = false;

    newRootNode->onUpdatePre([&newRootUpdated] { newRootUpdated = true; });

    window.setRootNode(std::move(replacementRootNode));
    window.updatePre();

    EXPECT_EQ(window.getRootNode(), newRootNode);
    EXPECT_FALSE(oldRootUpdated);
    EXPECT_TRUE(newRootUpdated);
}