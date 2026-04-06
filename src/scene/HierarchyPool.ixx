module;

#include <algorithm>
#include <functional>
#include <vector>

export module awen.scene.hierarchy_pool;

export import awen.scene.node_id;
export import awen.scene.node_pool;

export namespace awen::scene
{
    /// @brief Parent/child/sibling links for one node in the scene tree.
    ///
    /// Stored in a NodePool<HierarchyNode> parallel to any data pools, indexed
    /// by the same NodeId. Siblings are kept in ascending local_z order.
    struct HierarchyNode
    {
        NodeId parent{};
        NodeId firstChild{};
        NodeId nextSibling{};
        NodeId prevSibling{};
        int localZ{};
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
        [[nodiscard]] auto allocate(NodeId parent, int localZ = 0) -> NodeId
        {
            const auto id = pool_.allocate();
            auto* node = pool_.get(id);
            node->parent = parent;
            node->localZ = localZ;

            insertChild(parent, id);

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
        auto depthFirst(const F& visitor) const -> void
        {
            visitChildren(root_, visitor);
        }

        /// @brief Calls visitor(NodeId) for every live node in the subtree rooted at start.
        /// @param start NodeId of the subtree root. Its own id is not passed to visitor.
        /// @param visitor Callable invoked with the NodeId of each visited node.
        template <typename F>
        auto depthFirstFrom(NodeId start, const F& visitor) const -> void
        {
            visitChildren(start, visitor);
        }

    private:
        /// @brief Inserts child into parent's child list, maintaining ascending local_z order.
        auto insertChild(NodeId parentId, NodeId childId) -> void // NOLINT(bugprone-easily-swappable-parameters)
        {
            auto* parentNode = pool_.get(parentId);
            auto* childNode = pool_.get(childId);
            if (parentNode == nullptr || childNode == nullptr)
            {
                return;
            }

            const auto childZ = childNode->localZ;

            // Walk the sibling list to find the insertion point.
            auto prev = NodeId{};
            auto curr = parentNode->firstChild;

            while (curr.isValid())
            {
                const auto* currNode = pool_.get(curr);
                if (currNode == nullptr || currNode->localZ > childZ)
                {
                    break;
                }

                prev = curr;
                curr = currNode->nextSibling;
            }

            // Link child between prev and curr.
            childNode->prevSibling = prev;
            childNode->nextSibling = curr;

            if (prev.isValid())
            {
                pool_.get(prev)->nextSibling = childId;
            }
            else
            {
                parentNode->firstChild = childId;
            }

            if (curr.isValid())
            {
                pool_.get(curr)->prevSibling = childId;
            }
        }

        /// @brief Removes a node from its parent's sibling list without freeing the slot.
        auto detach(HierarchyNode& node) -> void
        {
            if (node.prevSibling.isValid())
            {
                pool_.get(node.prevSibling)->nextSibling = node.nextSibling;
            }
            else if (node.parent.isValid())
            {
                auto* parentNode = pool_.get(node.parent);
                if (parentNode != nullptr)
                {
                    parentNode->firstChild = node.nextSibling;
                }
            }

            if (node.nextSibling.isValid())
            {
                pool_.get(node.nextSibling)->prevSibling = node.prevSibling;
            }

            node.prevSibling = {};
            node.nextSibling = {};
            node.parent = {};
        }

        template <typename F>
        auto visitChildren(NodeId parentId, const F& visitor) const -> void
        {
            // Iterative depth-first traversal. The stack holds nodes yet to be visited.
            // Children are appended in sibling order then the newly added slice is reversed
            // in-place so the first child (lowest local_z) is always on top of the stack.

            // Pushes all siblings reachable from node->first_child onto the stack,
            // then reverses the newly added slice so the lowest-z sibling is on top.
            const auto pushChildren = [this](std::vector<NodeId>& stack, NodeId id)
            {
                const auto* node = pool_.get(id);

                if (node == nullptr)
                {
                    return;
                }

                const auto base = static_cast<std::ptrdiff_t>(std::size(stack));

                for (auto curr = node->firstChild; curr.isValid();)
                {
                    const auto* child = pool_.get(curr);
                    if (child == nullptr)
                    {
                        break;
                    }
                    stack.push_back(curr);
                    curr = child->nextSibling;
                }

                std::ranges::reverse(std::next(std::begin(stack), base), std::end(stack));
            };

            auto stack = std::vector<NodeId>{};
            pushChildren(stack, parentId);

            while (!std::empty(stack))
            {
                const auto id = stack.back();
                stack.pop_back();

                visitor(id);

                pushChildren(stack, id);
            }
        }

        NodePool<HierarchyNode> pool_;
        NodeId root_{};
    };
}
