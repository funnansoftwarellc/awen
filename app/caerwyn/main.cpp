#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
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

    constexpr std::uint8_t FullAlpha = 255;
    constexpr std::uint8_t GrayChannel = 160;
    constexpr std::uint8_t WhiteChannel = 240;
    constexpr std::uint8_t BlackChannel = 0;

    constexpr auto AliceColor = awen::widget::Color{.r = 230, .g = 110, .b = 130, .a = FullAlpha};
    constexpr auto BobColor = awen::widget::Color{.r = 110, .g = 200, .b = 230, .a = FullAlpha};
    constexpr auto CharlieColor = awen::widget::Color{.r = 180, .g = 230, .b = 130, .a = FullAlpha};
    constexpr auto GrayColor = awen::widget::Color{.r = GrayChannel, .g = GrayChannel, .b = GrayChannel, .a = FullAlpha};
    constexpr auto WhiteColor = awen::widget::Color{.r = WhiteChannel, .g = WhiteChannel, .b = WhiteChannel, .a = FullAlpha};
    constexpr auto BlackColor = awen::widget::Color{.r = BlackChannel, .g = BlackChannel, .b = BlackChannel, .a = FullAlpha};

    constexpr auto MessageOffset1 = 8;
    constexpr auto MessageOffset2 = 7;
    constexpr auto MessageOffset3 = 6;
    constexpr auto MessageOffset4 = 5;
    constexpr auto MessageOffset5 = 4;
    constexpr auto MessageOffset6 = 3;
    constexpr auto MessageOffset7 = 2;
    constexpr auto MessageOffset8 = 1;

    struct ChatMessage
    {
        std::chrono::system_clock::time_point timestamp;
        std::string username;
        awen::widget::Color color;
        std::string text;
    };

    auto formatTimestamp(std::chrono::system_clock::time_point tp) -> std::string
    {
        const auto daypoint = std::chrono::floor<std::chrono::days>(tp);
        const auto timeOfDay = std::chrono::hh_mm_ss{tp - daypoint};
        return std::format("{:02}:{:02}", timeOfDay.hours().count(), timeOfDay.minutes().count());
    }

    auto buildMessages() -> std::vector<ChatMessage>
    {
        const auto now = std::chrono::system_clock::now();
        return std::vector<ChatMessage>{
            {.timestamp = now - std::chrono::minutes(MessageOffset1),
             .username = "Alice",
             .color = AliceColor,
             .text = "Welcome to the Awen port of Caerwyn! This is a long enough message to test that "
                     "word wrapping is working as intended on the rich-text label."},
            {.timestamp = now - std::chrono::minutes(MessageOffset2),
             .username = "Bob",
             .color = BobColor,
             .text = "Looks like the scrollbar should appear once we add enough lines."},
            {.timestamp = now - std::chrono::minutes(MessageOffset3),
             .username = "Charlie",
             .color = CharlieColor,
             .text = "Try resizing the window. The view should re-flow live during the modal resize loop."},
            {.timestamp = now - std::chrono::minutes(MessageOffset4),
             .username = "Alice",
             .color = AliceColor,
             .text = "Each message uses a column layout with three runs: a gray timestamp, a bold-white username, "
                     "and the actual message body in white."},
            {.timestamp = now - std::chrono::minutes(MessageOffset5),
             .username = "Bob",
             .color = BobColor,
             .text = "Mouse wheel scrolling should also work over the viewport area."},
            {.timestamp = now - std::chrono::minutes(MessageOffset6),
             .username = "Charlie",
             .color = CharlieColor,
             .text = "And dragging the thumb of the scrollbar updates the offset too."},
            {.timestamp = now - std::chrono::minutes(MessageOffset7),
             .username = "Alice",
             .color = AliceColor,
             .text = "Hopefully the layout converges quickly enough to feel responsive."},
            {.timestamp = now - std::chrono::minutes(MessageOffset8),
             .username = "Bob",
             .color = BobColor,
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
    window->setClearColor(BlackColor);
    window->setUseLogicalPresentation(false);

    auto scrollView = std::make_unique<ScrollView>();
    auto* scrollViewNode = scrollView.get();
    scrollViewNode->setFillWindowOnResize(true);

    auto column = std::make_unique<ColumnLayout>();
    column->setSpacing(MessageSpacing);
    column->setPadding(Insets::all(MessagePadding));

    for (const auto& msg : buildMessages())
    {
        auto label = std::make_unique<RichTextLabel>();
        label->setWrapMode(WrapMode::Word);
        label->setLineSpacing(MessageLineSpacing);
        label->setRuns(std::vector<TextRun>{
            TextRun{.text = formatTimestamp(msg.timestamp) + " ", .font = fontRegular, .fontSize = MessageFontSize, .color = GrayColor},
            TextRun{.text = msg.username + ": ", .font = fontBold, .fontSize = MessageFontSize, .color = msg.color},
            TextRun{.text = msg.text, .font = fontRegular, .fontSize = MessageFontSize, .color = WhiteColor},
        });
        column->addWidget(std::move(label));
    }

    scrollViewNode->setContent(std::move(column));

    window->addChild(std::move(scrollView));

    engine.addChild(std::move(window));
    const auto result = engine.run();

    if (const auto& error = windowNode->getLastError(); error.has_value())
    {
        try
        {
            std::println(stderr, "caerwyn: {}", *error);
        }
        // NOLINTNEXTLINE(bugprone-empty-catch)
        catch (...)
        {
        }

        return EXIT_FAILURE;
    }

    return result;
}
catch (const std::exception& exception)
{
    try
    {
        std::println(stderr, "caerwyn: {}", exception.what());
    }
    // NOLINTNEXTLINE(bugprone-empty-catch)
    catch (...)
    {
    }

    return EXIT_FAILURE;
}
catch (...)
{
    try
    {
        std::println(stderr, "caerwyn: Unhandled non-standard exception.");
    }
    // NOLINTNEXTLINE(bugprone-empty-catch)
    catch (...)
    {
    }

    return EXIT_FAILURE;
}
