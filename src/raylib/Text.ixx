module;

#include <raylib.h>
#include <string>
#include <typeinfo>

export module awen.raylib.text;
import awen.core;
import awen.raylib.color;

export namespace awen::raylib
{
    class Text : public awen::core::Object
    {
    public:
        Text()
        {
            constexpr auto posX = 350;
            constexpr auto posY = 200;
            constexpr auto fontSize = 50;
            constexpr auto text = "Hello, Awen!";
            text_ = text;
            onRender([this] { DrawText(text_.c_str(), posX, posY, fontSize, ToRaylibColor(colors::Orange)); });
        }

        Text(const Text&) = delete;
        auto operator=(const Text&) -> Text& = delete;

        Text(Text&&) noexcept = delete;
        auto operator=(Text&&) noexcept -> Text& = delete;

        ~Text() override = default;

    private:
        std::string text_;
    };
}