module;

#include <cstdint>
#include <vector>

export module awen.scene.node_pool;

export import awen.scene.node_id;

export namespace awen::scene
{
    /// @brief Contiguous generational arena storing values of type T, indexed by NodeId.
    ///
    /// Generation encoding (no extra storage needed):
    ///   - 0         — slot never allocated (null sentinel)
    ///   - odd >= 1  — slot is alive
    ///   - even >= 2 — slot has been freed
    ///
    /// When a handle's generation differs from the stored generation, the slot
    /// has been freed or recycled — get() returns nullptr for stale handles.
    template <typename T>
    class NodePool
    {
    public:
        /// @brief Allocates a new slot, reusing a freed slot when available.
        /// @return A NodeId identifying the newly allocated slot.
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
            generations_.push_back(1U);
            return NodeId{.index = index, .generation = 1U};
        }

        /// @brief Activates a slot at exactly @p index, padding any gap with null sentinel slots.
        ///
        /// All skipped indices are inserted with generation 0 and are never returned as alive.
        /// This allows multiple pools to share a common index space driven by a single authoritative
        /// allocator (e.g. HierarchyPool): call allocate_at(N) on each secondary pool with the
        /// NodeId.index obtained from the primary pool to ensure all pools stay in step.
        ///
        /// @param index The slot index to activate.
        /// @return A NodeId for the newly activated slot.
        /// @note Only call with indices that have never been allocated. Activating a slot whose
        ///       index is already alive produces undefined behaviour.
        auto allocate_at(uint32_t index) -> NodeId
        {
            // Grow backing storage; padding slots use generation 0 (null sentinel, never alive).
            while (static_cast<uint32_t>(data_.size()) <= index)
            {
                data_.emplace_back();
                generations_.push_back(0U);
            }

            // Activate: generation 1 for a fresh slot, or odd(+1) for a recycled-then-reused slot.
            auto& gen = generations_[index];
            gen = (gen == 0U) ? 1U : gen + 1U;
            data_[index] = T{};
            return NodeId{.index = index, .generation = gen};
        }

        /// @brief Frees the slot identified by id.
        /// @param id The NodeId of the slot to free. Stale or null ids are silently ignored.
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

        /// @brief Returns a pointer to the value stored in the given slot.
        /// @param id The NodeId of the slot to retrieve.
        /// @return Pointer to the stored value, or nullptr if id is stale or null.
        [[nodiscard]] auto get(NodeId id) -> T*
        {
            if (!is_alive(id))
            {
                return nullptr;
            }
            return &data_[id.index];
        }

        /// @brief Returns a const pointer to the value stored in the given slot.
        /// @param id The NodeId of the slot to retrieve.
        /// @return Const pointer to the stored value, or nullptr if id is stale or null.
        [[nodiscard]] auto get(NodeId id) const -> const T*
        {
            if (!is_alive(id))
            {
                return nullptr;
            }
            return &data_[id.index];
        }

        /// @brief Returns the total number of backing slots, including freed slots.
        /// @note Use for_each to visit only live nodes.
        [[nodiscard]] auto capacity() const noexcept -> std::size_t
        {
            return data_.size();
        }

        /// @brief Calls fn(NodeId, T&) for every live slot in allocation order.
        /// @param fn Callable invoked with the NodeId and a mutable reference to each live value.
        template <typename F>
        auto for_each(const F& fn) -> void
        {
            const auto count = static_cast<uint32_t>(data_.size());
            for (auto i = uint32_t{0}; i < count; ++i)
            {
                // An odd generation indicates a live slot.
                if (generations_[i] & 1U)
                {
                    fn(NodeId{.index = i, .generation = generations_[i]}, data_[i]);
                }
            }
        }

        /// @brief Calls fn(NodeId, const T&) for every live slot in allocation order.
        /// @param fn Callable invoked with the NodeId and a const reference to each live value.
        template <typename F>
        auto for_each(const F& fn) const -> void
        {
            const auto count = static_cast<uint32_t>(data_.size());
            for (auto i = uint32_t{0}; i < count; ++i)
            {
                // An odd generation indicates a live slot.
                if (generations_[i] & 1U)
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
