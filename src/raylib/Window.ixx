module;

#include <GLFW/glfw3.h>
#include <raylib.h>
#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

export module awen.raylib.window;
import awen.core;
import awen.raylib.events;
import awen.raylib.color;
import awen.raylib.node;

export namespace awen::raylib
{
    inline constexpr std::string_view DefaultWindowTitle{"Awen"};
    inline constexpr auto DefaultWindowWidth{1280};
    inline constexpr auto DefaultWindowHeight{720};

    struct WindowTraits
    {
        std::string_view title{DefaultWindowTitle};
        int width{DefaultWindowWidth};
        int height{DefaultWindowHeight};
    };

    class Window : public awen::core::Object
    {
    public:
        using Traits = WindowTraits;

        Window(const Traits& traits = Traits{}) : title_{traits.title}, width_{traits.width}, height_{traits.height}

        {
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
            InitWindow(width_, height_, title_.c_str());

            onUpdatePre(
                [this]
                {
                    auto events = ProcessEvents();

                    for (auto& event : events)
                    {
                        rootNode_->events(event);
                    }

                    // Poll events for web assembly after processing events.
                    // Otherwise, it don't work.
                    PollInputEvents();
                });

            onUpdatePost(
                [this]
                {
                    if (WindowShouldClose())
                    {
                        auto* engine = rootNode_->getParent<awen::core::Engine>();

                        if (engine != nullptr)
                        {
                            engine->stop();
                        }
                    }

                    rootNode_->renderPre();

                    BeginDrawing();
                    ClearBackground(ToRaylibColor(color_));

                    rootNode_->render();

                    DrawFPS(0, 0);
                    EndDrawing();

                    SwapScreenBuffer();

                    rootNode_->renderPost();

                    const auto pos = GetWindowPosition();
                    posX_ = static_cast<int>(pos.x);
                    posY_ = static_cast<int>(pos.y);
                    width_ = GetScreenWidth();
                    height_ = GetScreenHeight();
                });
        }

        Window(const Window&) = delete;
        auto operator=(const Window&) -> Window& = delete;
        Window(Window&&) noexcept = delete;
        auto operator=(Window&&) noexcept -> Window& = delete;

        ~Window() override
        {
            CloseWindow();
        }

        auto setPosX(int x) -> void
        {
            posX_ = x;
            SetWindowPosition(posX_, getPosY());
        }

        [[nodiscard]] auto getPosX() const -> int
        {
            return posX_;
        }

        auto setPosY(int x) -> void
        {
            posY_ = x;
            SetWindowPosition(getPosX(), posY_);
        }

        [[nodiscard]] auto getPosY() const -> int
        {
            return posY_;
        }

        auto setWidth(int x) -> void
        {
            width_ = x;
            SetWindowSize(width_, getHeight());
        }

        [[nodiscard]] auto getWidth() const -> int
        {
            return width_;
        }

        auto setHeight(int x) -> void
        {
            height_ = x;
            SetWindowSize(getWidth(), height_);
        }

        [[nodiscard]] auto getHeight() const -> int
        {
            return height_;
        }

        auto setTitle(std::string_view title) -> void
        {
            title_ = title;
            SetWindowTitle(title_.c_str());
        }

        [[nodiscard]] auto getTitle() const noexcept -> std::string_view
        {
            return title_;
        }

        auto setColor(const Color& color) -> void
        {
            color_ = color;
        }

        [[nodiscard]] auto getColor() const noexcept -> Color
        {
            return color_;
        }

        auto setRootNode(std::unique_ptr<Node> node) -> void
        {
            if (node == nullptr)
            {
                return;
            }

            std::ignore = rootNode_->remove();

            rootNode_ = node.get();
            addChild(std::move(node));
        }

        [[nodiscard]] auto getRootNode() const -> Node*
        {
            return rootNode_;
        }

    private:
        std::string title_;
        int posX_{};
        int posY_{};
        int width_{};
        int height_{};
        awen::raylib::Color color_{colors::Black};
        awen::raylib::Node* rootNode_{addChild<Node>()};
    };
}