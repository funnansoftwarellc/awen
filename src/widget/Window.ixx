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

            onStartup().connect(
                [this]
                {
                    const auto width = static_cast<int>(getSize()[0]);
                    const auto height = static_cast<int>(getSize()[1]);
                    window_ = std::make_unique<awen::graphics::Window>(title_.c_str(), width, height,
                                                                       std::initializer_list<ConfigFlag>{ConfigFlag::resizable});
                });

            Engine::instance()->onEvent().connect([this] { window_->pollEvents(); });

            Engine::instance()->onRender().connect(
                []
                {
                    using awen::graphics::Renderer;

                    Renderer::begin();
                    Renderer::clear(awen::graphics::colors::Orange);
                    Renderer::end();
                });
        }

        auto setTitle(std::string_view title) -> void
        {
            title_ = title;
        }

        [[nodiscard]] auto getTitle() const -> std::string_view
        {
            return title_;
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
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