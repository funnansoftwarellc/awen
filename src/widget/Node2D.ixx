module;

#include <glm/vec2.hpp>

export module awen.widget.node2d;
import awen.widget.node;

export namespace awen::widget
{
    class Node2D : public awen::widget::Node
    {
    public:
        auto set_position(glm::vec2 x) -> void
        {
            position_ = x;
        }

        auto get_position() const -> glm::vec2
        {
            return position_;
        }

    private:
        glm::vec2 position_{};
    };

}
