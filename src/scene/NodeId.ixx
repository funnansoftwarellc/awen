module;

#include <cstdint>

export module awen.scene.node_id;

export namespace awen::scene
{
    /// @brief Stable handle to a slot in a NodePool.
    ///
    /// The generation field distinguishes a live slot from a recycled one.
    /// A NodeId with generation 0 is always invalid (the null sentinel).
    struct NodeId
    {
        uint32_t index{};
        uint32_t generation{};

        /// @brief Returns true when the id refers to a potentially live slot.
        /// @note A valid id is not guaranteed to be alive — it may refer to a
        ///       freed slot if the owning pool has not yet recycled the index.
        [[nodiscard]] auto isValid() const noexcept -> bool
        {
            return generation != 0;
        }

        auto operator==(const NodeId&) const noexcept -> bool = default;
    };

    /// @brief Sentinel value representing an absent or unset node reference.
    inline constexpr auto NullNode = NodeId{};
}
