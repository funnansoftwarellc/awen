#include <raylib.h>
#include <array>
#include <cstdlib>
#include <format>

// NOLINTBEGIN
#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 512
#include <magic_enum/magic_enum.hpp>
// NOLINTEND
#include <tuple>
#include <typeinfo>
#include <variant>

import awen.core;
import awen.raylib;

using awen::raylib::Window;

auto main() -> int
{
    constexpr auto width{1280};
    constexpr auto height{720};

    awen::core::Engine engine;

    auto* window = engine.addChild<Window>(Window::Traits{
        .title = "Awen",
        .width = width,
        .height = height,
    });

    auto* rootNode = window->getRootNode();

    std::ignore = rootNode->addNode<awen::raylib::TextFPS>();

    // NOLINTBEGIN
    auto* rectOne = rootNode->addNode<awen::raylib::Rectangle>();
    rectOne->setPosition({.x = 100.0F, .y = 100.0F});
    rectOne->setRotation(15.0F);
    rectOne->setScale({.x = 1.5F, .y = 1.0F});
    rectOne->setWidth(200.0F);
    rectOne->setHeight(200.0F);
    rectOne->setColor(awen::raylib::colors::Red);

    auto* rectOneChild = rectOne->addNode<awen::raylib::Rectangle>();
    rectOneChild->setPosition({.x = 10.0F, .y = 10.0F});
    rectOneChild->setWidth(50.0F);
    rectOneChild->setHeight(50.0F);
    rectOneChild->setColor(awen::raylib::colors::Blue);

    auto* groupText = rootNode->addNode<awen::raylib::Node>();

    groupText->setPosition({.x = width / 2.0F, .y = height / 2.0F});

    auto* txtKeys = groupText->addNode<awen::raylib::Text>();
    txtKeys->setText("Press any key...");
    auto* txtMouse = groupText->addNode<awen::raylib::Text>();
    txtMouse->setPosition({.x = 0.0F, .y = 50.0F});
    txtMouse->setText("Move the mouse...");

    auto* ring = rootNode->addNode<awen::raylib::Ring>();
    ring->setPosition({.x = width / 2.0F, .y = height / 2.0F});
    ring->setOuterRadius(height / 2.0F);
    ring->setInnerRadius(ring->getOuterRadius() - 4.0F);
    ring->setStartAngle(0.0F);
    ring->setEndAngle(330.0F);
    ring->setSegments(90);
    ring->setColor(awen::raylib::colors::White);

    auto* triangleStrip = rootNode->addNode<awen::raylib::TriangleStrip>();
    triangleStrip->setVertices({
        {.x = 0.0F, .y = -0.5F},
        {.x = -0.5F, .y = 0.5F},
        {.x = 0.5F, .y = 0.5F},
    });
    triangleStrip->setColor(awen::raylib::colors::Green);
    triangleStrip->setScale({.x = 64.0F, .y = 64.0F});
    triangleStrip->setPosition({.x = width / 2.0F, .y = height / 2.0F});
    // NOLINTEND

    rootNode->onEvents(
        [txtKeys, txtMouse, rectOne, rectOneChild, colorIndex = std::size_t{0}](const auto& event) mutable
        {
            if (std::holds_alternative<awen::raylib::EventKeyboard>(event))
            {
                const auto& keyboardEvent = std::get<awen::raylib::EventKeyboard>(event);

                if (keyboardEvent.type == awen::raylib::EventKeyboard::Type::Pressed)
                {
                    txtKeys->setText(std::format("Key {} pressed", magic_enum::enum_name(keyboardEvent.key)));
                }

                if (keyboardEvent.type == awen::raylib::EventKeyboard::Type::Released)
                {
                    txtKeys->setText(std::format("Key {} released", magic_enum::enum_name(keyboardEvent.key)));
                }
            }

            if (std::holds_alternative<awen::raylib::EventMouse>(event))
            {
                const auto& mouseEvent = std::get<awen::raylib::EventMouse>(event);

                if (mouseEvent.type == awen::raylib::EventMouse::Type::ButtonPressed && mouseEvent.button == awen::raylib::EventMouse::Button::Left)
                {
                    constexpr auto cycleColors = std::array{
                        awen::raylib::colors::Red,  awen::raylib::colors::Yellow, awen::raylib::colors::Magenta,
                        awen::raylib::colors::Cyan, awen::raylib::colors::Orange,
                    };

                    colorIndex = (colorIndex + 1) % std::size(cycleColors);
                    rectOne->setColor(cycleColors.at(colorIndex));
                    rectOneChild->setColor(cycleColors.at((colorIndex + 1) % std::size(cycleColors)));
                }

                if (mouseEvent.type == awen::raylib::EventMouse::Type::Moved)
                {
                    txtMouse->setText(std::format("Mouse moved to ({}, {})", mouseEvent.x, mouseEvent.y));
                }
            }
        });

    return engine.run();
}
