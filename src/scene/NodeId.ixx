module;

#include <cstdint>

export module awen.scene.node_id;

export namespace awn::scene
{
    struct NodeId
    {
        uint32_t index{};
        uint32_t generation{};

        [[nodiscard]] auto is_valid() const noexcept -> bool
        {
            return generation != 0;
        }

        auto operator==(const NodeId&) const noexcept -> bool = default;
    };

    inline constexpr auto null_node = NodeId{};
}
