#include <chrono>
#include <cstdlib>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <utility>
#include <variant>
#include <vector>

import awen.core;
import awen.sdl.font;
import awen.widget;

namespace
{
    constexpr auto WindowWidth = 720.0F;
    constexpr auto WindowHeight = 1440.0F;
    constexpr auto WindowPositionX = 80.0F;
    constexpr auto WindowPositionY = 60.0F;

    constexpr auto MessageFontSize = 24;
    constexpr auto MessagePadding = 10.0F;
    constexpr auto MessageSpacing = 6.0F;
    constexpr auto MessageLineSpacing = 8.0F;

    struct ChatMessage
    {
        std::chrono::system_clock::time_point timestamp;
        std::string username;
        awen::widget::Color color;
        std::string text;
    };

    auto formatTimestamp(std::chrono::system_clock::time_point tp) -> std::string
    {
        const auto local = std::chrono::zoned_time{std::chrono::current_zone(), tp};
        return std::format("{:%H:%M}", local);
    }

    auto buildMessages() -> std::vector<ChatMessage>
    {
        const auto now = std::chrono::system_clock::now();
        return std::vector<ChatMessage>{
            {.timestamp = now - std::chrono::minutes(8),
             .username = "Alice",
             .color = awen::widget::Color{.r = 230, .g = 110, .b = 130, .a = 255},
             .text = "Welcome to the Awen port of Caerwyn! This is a long enough message to test that "
                     "word wrapping is working as intended on the rich-text label."},
            {.timestamp = now - std::chrono::minutes(7),
             .username = "Bob",
             .color = awen::widget::Color{.r = 110, .g = 200, .b = 230, .a = 255},
             .text = "Looks like the scrollbar should appear once we add enough lines."},
            {.timestamp = now - std::chrono::minutes(6),
             .username = "Charlie",
             .color = awen::widget::Color{.r = 180, .g = 230, .b = 130, .a = 255},
             .text = "Try resizing the window. The view should re-flow live during the modal resize loop."},
            {.timestamp = now - std::chrono::minutes(5),
             .username = "Alice",
             .color = awen::widget::Color{.r = 230, .g = 110, .b = 130, .a = 255},
             .text = "Each message uses a column layout with three runs: a gray timestamp, a bold-white username, "
                     "and the actual message body in white."},
            {.timestamp = now - std::chrono::minutes(4),
             .username = "Bob",
             .color = awen::widget::Color{.r = 110, .g = 200, .b = 230, .a = 255},
             .text = "Mouse wheel scrolling should also work over the viewport area."},
            {.timestamp = now - std::chrono::minutes(3),
             .username = "Charlie",
             .color = awen::widget::Color{.r = 180, .g = 230, .b = 130, .a = 255},
             .text = "And dragging the thumb of the scrollbar updates the offset too."},
            {.timestamp = now - std::chrono::minutes(2),
             .username = "Alice",
             .color = awen::widget::Color{.r = 230, .g = 110, .b = 130, .a = 255},
             .text = "Hopefully the layout converges quickly enough to feel responsive."},
            {.timestamp = now - std::chrono::minutes(1),
             .username = "Bob",
             .color = awen::widget::Color{.r = 110, .g = 200, .b = 230, .a = 255},
             .text = "If you can read this last message at the bottom, scrolling works end-to-end."},
        };
    }
}

auto main() -> int
try
{
    using awen::widget::ColumnLayout;
    using awen::widget::Insets;
    using awen::widget::Rect;
    using awen::widget::RichTextLabel;
    using awen::widget::ScrollView;
    using awen::widget::TextRun;
    using awen::widget::Window;
    using awen::widget::WrapMode;

    const auto fontRegular = awen::sdl::FontHandle{.path = "Roboto-Regular.ttf", .sizePx = MessageFontSize};
    const auto fontBold = awen::sdl::FontHandle{.path = "Roboto-Bold.ttf", .sizePx = MessageFontSize};

    auto engine = awen::core::Engine{};

    auto window = std::make_unique<Window>();
    auto* windowNode = window.get();
    window->setTitle("Caerwyn");
    window->setSize({WindowWidth, WindowHeight});
    window->setPosition({WindowPositionX, WindowPositionY});
    window->setClearColor(awen::widget::Color{.r = 0, .g = 0, .b = 0, .a = 255});
    window->setUseLogicalPresentation(false);

    auto scrollView = std::make_unique<ScrollView>();
    auto* scrollViewNode = scrollView.get();
    scrollViewNode->setFillWindowOnResize(true);

    auto column = std::make_unique<ColumnLayout>();
    column->setSpacing(MessageSpacing);
    column->setPadding(Insets::all(MessagePadding));

    const auto gray = awen::widget::Color{.r = 160, .g = 160, .b = 160, .a = 255};
    const auto white = awen::widget::Color{.r = 240, .g = 240, .b = 240, .a = 255};

    for (const auto& msg : buildMessages())
    {
        auto label = std::make_unique<RichTextLabel>();
        label->setWrapMode(WrapMode::Word);
        label->setLineSpacing(MessageLineSpacing);
        label->setRuns(std::vector<TextRun>{
            TextRun{.text = formatTimestamp(msg.timestamp) + " ", .font = fontRegular, .fontSize = MessageFontSize, .color = gray},
            TextRun{.text = msg.username + ": ", .font = fontBold, .fontSize = MessageFontSize, .color = msg.color},
            TextRun{.text = msg.text, .font = fontRegular, .fontSize = MessageFontSize, .color = white},
        });
        column->addWidget(std::move(label));
    }

    scrollViewNode->setContent(std::move(column));

    window->addChild(std::move(scrollView));

    engine.addChild(std::move(window));
    const auto result = engine.run();

    if (const auto& error = windowNode->getLastError(); error.has_value())
    {
        std::println(stderr, "caerwyn: {}", *error);
        return EXIT_FAILURE;
    }

    return result;
}
catch (const std::exception& exception)
{
    std::println(stderr, "caerwyn: {}", exception.what());
    return EXIT_FAILURE;
}
catch (...)
{
    std::println(stderr, "caerwyn: Unhandled non-standard exception.");
    return EXIT_FAILURE;
}
