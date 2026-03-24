module;

#include <cstdint>
#include <vector>

export module awen.scene.node_pool;

export import awen.scene.node_id;

export namespace awn::scene
{
    // NodePool<T> stores T values in a contiguous array indexed by NodeId.
    //
    // Generation encoding (no extra storage needed):
    //   0          — slot never allocated (null sentinel)
    //   odd  >= 1  — slot is alive
    //   even >= 2  — slot has been freed
    //
    // When a handle's generation differs from the stored generation, the slot
    // has been freed or recycled — get() returns nullptr for stale handles.
    template <typename T>
    class NodePool
    {
    public:
        [[nodiscard]] auto allocate() -> NodeId
        {
            if (!free_list_.empty())
            {
                const auto index = free_list_.back();
                free_list_.pop_back();

                // Incrementing an even (dead) generation produces an odd (alive) generation.
                ++generations_[index];
                data_[index] = T{};
                return NodeId{.index = index, .generation = generations_[index]};
            }

            const auto index = static_cast<uint32_t>(data_.size());
            data_.emplace_back();

            // Generation 1 is the first live allocation for a new slot.
            generations_.push_back(1u);
            return NodeId{.index = index, .generation = 1u};
        }

        auto free(NodeId id) -> void
        {
            if (!is_alive(id))
            {
                return;
            }

            // Incrementing an odd (alive) generation produces an even (dead) generation.
            ++generations_[id.index];
            free_list_.push_back(id.index);
        }

        [[nodiscard]] auto get(NodeId id) -> T*
        {
            if (!is_alive(id))
            {
                return nullptr;
            }
            return &data_[id.index];
        }

        [[nodiscard]] auto get(NodeId id) const -> const T*
        {
            if (!is_alive(id))
            {
                return nullptr;
            }
            return &data_[id.index];
        }

        // Returns the number of backing slots, including both live and freed slots.
        // Use for_each to visit only live nodes.
        [[nodiscard]] auto capacity() const noexcept -> std::size_t
        {
            return data_.size();
        }

        template <typename F>
        auto for_each(F&& fn) -> void
        {
            const auto count = static_cast<uint32_t>(data_.size());
            for (auto i = uint32_t{0}; i < count; ++i)
            {
                // An odd generation indicates a live slot.
                if (generations_[i] & 1u)
                {
                    fn(NodeId{.index = i, .generation = generations_[i]}, data_[i]);
                }
            }
        }

        template <typename F>
        auto for_each(F&& fn) const -> void
        {
            const auto count = static_cast<uint32_t>(data_.size());
            for (auto i = uint32_t{0}; i < count; ++i)
            {
                // An odd generation indicates a live slot.
                if (generations_[i] & 1u)
                {
                    fn(NodeId{.index = i, .generation = generations_[i]}, data_[i]);
                }
            }
        }

    private:
        [[nodiscard]] auto is_alive(NodeId id) const noexcept -> bool
        {
            return id.is_valid() && id.index < static_cast<uint32_t>(generations_.size()) && generations_[id.index] == id.generation;
        }

        std::vector<T> data_;
        std::vector<uint32_t> generations_;
        std::vector<uint32_t> free_list_;
    };
}
