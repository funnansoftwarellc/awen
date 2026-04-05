module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

export module awen.widget.window;

import awen.core.engine;
import awen.graphics.window;
import awen.graphics.renderer;
import awen.graphics.color;
import awen.widget.control;
import awen.widget.components;

using awen::core::Engine;

export namespace awen::widget
{
    class Window : public Control
    {
    public:
        Window()
        {
            using awen::core::Engine;
            using awen::graphics::ConfigFlag;
            using awen::graphics::Window;

            on_startup.connect(
                [this]
                {
                    const auto width = static_cast<int>(get_size().x);
                    const auto height = static_cast<int>(get_size().y);
                    window_ = std::make_unique<awen::graphics::Window>(title_.c_str(), width, height,
                                                                       std::initializer_list<ConfigFlag>{ConfigFlag::resizable});
                });

            Engine::instance()->on_event.connect([this] { window_->poll_events(); });

            Engine::instance()->on_render.connect(
                []
                {
                    using awen::graphics::Renderer;

                    Renderer::begin();
                    Renderer::clear(awen::graphics::colors::orange);
                    Renderer::end();
                });
        }

        auto set_title(std::string_view title) -> void
        {
            title_ = title;
        }

        [[nodiscard]] auto get_title() const -> std::string_view
        {
            return title_;
        }

        auto synchronize(flecs::entity entity) const -> flecs::entity
        {
            using awen::widget::components::Transform;

            entity = Control::synchronize(entity);

            auto& transform = entity.get_mut<Transform>();
            transform.position = glm::vec2{0.0F, 0.0F};

            return entity;
        }

    private:
        std::unique_ptr<awen::graphics::Window> window_;
        std::string title_;
    };
}