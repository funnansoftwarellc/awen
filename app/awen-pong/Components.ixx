module;

#include <glm/vec2.hpp>

export module awen.pong.components;

export namespace pong
{
    /// @brief Per-side paddle state.
    struct Paddle
    {
        float y{};
        int score{};
    };

    /// @brief Ball position + velocity.
    struct Ball
    {
        glm::vec2 position{};
        glm::vec2 velocity{};
    };

    /// @brief Singleton game state owned by the world.
    struct PongState
    {
        Paddle leftPad;
        Paddle rightPad;
        Ball ball;
        bool p2Ai{true};
    };

    struct LeftPaddleTag
    {
    };

    struct RightPaddleTag
    {
    };

    struct BallTag
    {
    };

    struct LeftScoreTag
    {
    };

    struct RightScoreTag
    {
    };

    struct LeftHintTag
    {
    };

    struct RightHintTag
    {
    };

    /// @brief Tag for centre-line dashes; index drives vertical position.
    struct DashTag
    {
        int index{};
    };
}
