export module awen.scene.transform;

export namespace awn::scene
{
    /// @brief Local (parent-relative) 2D position of a scene node.
    struct Transform
    {
        float x{};
        float y{};
    };

    /// @brief Accumulated world-space 2D position of a scene node.
    ///
    /// Recomputed each frame during the traversal pass by summing a node's
    /// Transform with the WorldTransform of every ancestor up to the root.
    struct WorldTransform
    {
        float x{};
        float y{};
    };
}
