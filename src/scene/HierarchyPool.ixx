module;

#include <algorithm>
#include <functional>
#include <vector>

export module awen.scene.hierarchy_pool;

export import awen.scene.node_id;
export import awen.scene.node_pool;

export namespace awn::scene
{
    /// @brief Parent/child/sibling links for one node in the scene tree.
    ///
    /// Stored in a NodePool<HierarchyNode> parallel to any data pools, indexed
    /// by the same NodeId. Siblings are kept in ascending local_z order.
    struct HierarchyNode
    {
        NodeId parent{};
        NodeId first_child{};
        NodeId next_sibling{};
        NodeId prev_sibling{};
        int local_z{};
    };

    /// @brief Owns the parent/child/sibling relationships for the entire scene tree.
    ///
    /// Wraps NodePool<HierarchyNode> and provides attach/detach helpers that keep
    /// all sibling links consistent. A root sentinel node is allocated on construction
    /// and represents the invisible top of the tree; it is never passed to visitors.
    class HierarchyPool
    {
    public:
        /// @brief Constructs the pool and allocates the root sentinel node.
        HierarchyPool()
        {
            root_ = pool_.allocate();
        }

        /// @brief Returns the NodeId of the root sentinel node.
        [[nodiscard]] auto root() const noexcept -> NodeId
        {
            return root_;
        }

        /// @brief Allocates a new hierarchy slot and attaches it as a child of parent.
        /// @param parent NodeId of the parent node.
        /// @param local_z Draw order relative to siblings; lower values are visited first.
        /// @return NodeId of the newly allocated node.
        [[nodiscard]] auto allocate(NodeId parent, int local_z = 0) -> NodeId
        {
            const auto id = pool_.allocate();
            auto* node = pool_.get(id);
            node->parent = parent;
            node->local_z = local_z;

            insert_child(parent, id);

            return id;
        }

        /// @brief Detaches the node from its parent and frees the hierarchy slot.
        /// @param id NodeId of the node to free. Null or stale ids are silently ignored.
        /// @note Children of the freed node are not reparented; callers must free
        ///       all descendants before freeing a parent.
        auto free(NodeId id) -> void
        {
            auto* node = pool_.get(id);
            if (node == nullptr)
            {
                return;
            }

            detach(*node);
            pool_.free(id);
        }

        /// @brief Returns a mutable pointer to the HierarchyNode for the given id.
        /// @param id NodeId of the node to retrieve.
        /// @return Pointer to the HierarchyNode, or nullptr if id is stale or null.
        [[nodiscard]] auto get(NodeId id) -> HierarchyNode*
        {
            return pool_.get(id);
        }

        /// @brief Returns a const pointer to the HierarchyNode for the given id.
        /// @param id NodeId of the node to retrieve.
        /// @return Const pointer to the HierarchyNode, or nullptr if id is stale or null.
        [[nodiscard]] auto get(NodeId id) const -> const HierarchyNode*
        {
            return pool_.get(id);
        }

        /// @brief Calls visitor(NodeId) for every live node in depth-first order.
        /// @param visitor Callable invoked with the NodeId of each visited node.
        /// @note Siblings are visited in ascending local_z order. The root sentinel is not visited.
        template <typename F>
        auto depth_first(F&& visitor) const -> void
        {
            visit_children(root_, visitor);
        }

        /// @brief Calls visitor(NodeId) for every live node in the subtree rooted at start.
        /// @param start NodeId of the subtree root. Its own id is not passed to visitor.
        /// @param visitor Callable invoked with the NodeId of each visited node.
        template <typename F>
        auto depth_first_from(NodeId start, F&& visitor) const -> void
        {
            visit_children(start, visitor);
        }

    private:
        /// @brief Inserts child into parent's child list, maintaining ascending local_z order.
        auto insert_child(NodeId parent_id, NodeId child_id) -> void
        {
            auto* parent_node = pool_.get(parent_id);
            auto* child_node = pool_.get(child_id);
            if (parent_node == nullptr || child_node == nullptr)
            {
                return;
            }

            const auto child_z = child_node->local_z;

            // Walk the sibling list to find the insertion point.
            auto prev = NodeId{};
            auto curr = parent_node->first_child;

            while (curr.is_valid())
            {
                const auto* curr_node = pool_.get(curr);
                if (curr_node == nullptr || curr_node->local_z > child_z)
                {
                    break;
                }

                prev = curr;
                curr = curr_node->next_sibling;
            }

            // Link child between prev and curr.
            child_node->prev_sibling = prev;
            child_node->next_sibling = curr;

            if (prev.is_valid())
            {
                pool_.get(prev)->next_sibling = child_id;
            }
            else
            {
                parent_node->first_child = child_id;
            }

            if (curr.is_valid())
            {
                pool_.get(curr)->prev_sibling = child_id;
            }
        }

        /// @brief Removes a node from its parent's sibling list without freeing the slot.
        auto detach(HierarchyNode& node) -> void
        {
            if (node.prev_sibling.is_valid())
            {
                pool_.get(node.prev_sibling)->next_sibling = node.next_sibling;
            }
            else if (node.parent.is_valid())
            {
                auto* parent_node = pool_.get(node.parent);
                if (parent_node != nullptr)
                {
                    parent_node->first_child = node.next_sibling;
                }
            }

            if (node.next_sibling.is_valid())
            {
                pool_.get(node.next_sibling)->prev_sibling = node.prev_sibling;
            }

            node.prev_sibling = {};
            node.next_sibling = {};
            node.parent = {};
        }

        template <typename F>
        auto visit_children(NodeId parent_id, F&& visitor) const -> void
        {
            const auto* parent_node = pool_.get(parent_id);
            if (parent_node == nullptr)
            {
                return;
            }

            // Siblings are already sorted by local_z at insert time, so a forward
            // walk produces the correct depth-first draw order.
            auto curr = parent_node->first_child;
            while (curr.is_valid())
            {
                const auto* curr_node = pool_.get(curr);
                if (curr_node == nullptr)
                {
                    break;
                }

                visitor(curr);
                visit_children(curr, visitor);

                curr = curr_node->next_sibling;
            }
        }

        NodePool<HierarchyNode> pool_;
        NodeId root_{};
    };
}
