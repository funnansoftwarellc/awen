#include <raylib.h>
#include <cstdlib>
#include <format>
#include <magic_enum/magic_enum.hpp>
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

    // NOLINTBEGIN
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

    auto* pos = rootNode->addNode<awen::raylib::Node>();

    pos->setPosition({.x = width / 2.0F, .y = height / 2.0F});
    // NOLINTEND
    
    auto* text = pos->addNode<awen::raylib::Text>();

    rootNode->onEvents(
        [text](const auto& event)
        {
            if (std::holds_alternative<awen::raylib::EventKeyboard>(event))
            {
                const auto& keyboardEvent = std::get<awen::raylib::EventKeyboard>(event);

                if (keyboardEvent.type == awen::raylib::EventKeyboard::Type::Pressed)
                {
                    text->setText(std::format("Key {} pressed", magic_enum::enum_name(keyboardEvent.key)));
                }

                if (keyboardEvent.type == awen::raylib::EventKeyboard::Type::Released)
                {
                    text->setText(std::format("Key {} released", magic_enum::enum_name(keyboardEvent.key)));
                }
            }
        });

    return engine.run();
}