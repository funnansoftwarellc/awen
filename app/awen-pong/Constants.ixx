module;

#include <numbers>

export module awen.pong.constants;

export namespace pong
{
    inline constexpr auto InitWidth = 1280;
    inline constexpr auto InitHeight = 720;
    inline constexpr auto WindowPositionX = 80;
    inline constexpr auto WindowPositionY = 80;

    inline constexpr auto PaddleWidth = 14.0F;
    inline constexpr auto PaddleHeight = 90.0F;
    inline constexpr auto PaddleSpeed = 380.0F;
    inline constexpr auto PaddleOffset = 128.0F;

    inline constexpr auto BallRadius = 10.0F;
    inline constexpr auto BallInitSpeed = 300.0F;
    inline constexpr auto BallSpeedUp = 1.05F;
    inline constexpr auto BallMaxSpeed = 700.0F;
    inline constexpr auto MaxBounceAngle = 60.0F;
    inline constexpr auto BallLaunchAngle = 30;

    inline constexpr auto AiSpeedRatio = 0.85F;
    inline constexpr auto Half = 0.5F;
    inline constexpr auto Deg2Rad = std::numbers::pi_v<float> / 180.0F;

    inline constexpr auto DashWidth = 4.0F;
    inline constexpr auto DashHeight = 10.0F;
    inline constexpr auto DashGap = 20;
    inline constexpr auto MaxDashes = 220;

    inline constexpr auto ScoreFontSize = 52;
    inline constexpr auto HintFontSize = 16;
    inline constexpr auto ScoreOffsetFromCenter = 70.0F;
    inline constexpr auto ScoreY = 18.0F;
    inline constexpr auto HintMargin = 10.0F;
    inline constexpr auto HintYFromBottom = 22.0F;
}
