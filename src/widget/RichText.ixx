module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

export module awen.widget.richtext;

import awen.core.object;
import awen.sdl.font;
import awen.sdl.renderer;
import awen.widget.color;
import awen.widget.nodetext;
import awen.widget.nodetransform;
import awen.widget.widget;

export namespace awen::widget
{
    /// @brief Text wrapping strategy.
    enum class WrapMode : std::uint8_t
    {
        None,
        Word,
    };

    /// @brief A styled run of text within a `RichTextLabel`.
    struct TextRun
    {
        std::string text;
        awen::sdl::FontHandle font{};
        int fontSize{16};
        Color color{colors::White};
    };

    /// @brief A multi-run, word-wrapped text widget.  Each run can have its
    /// own font/size/color.  The label measures and arranges itself based on
    /// the available width and re-emits child `NodeText` fragments via
    /// `NodeTransform`s when the layout changes.
    class RichTextLabel : public Widget
    {
    public:
        auto setRuns(std::vector<TextRun> runs) -> void
        {
            runs_ = std::move(runs);
            invalidate();
        }

        auto addRun(TextRun run) -> void
        {
            runs_.push_back(std::move(run));
            invalidate();
        }

        [[nodiscard]] auto getRuns() const -> const std::vector<TextRun>&
        {
            return runs_;
        }

        auto setWrapMode(WrapMode mode) -> void
        {
            if (wrapMode_ != mode)
            {
                wrapMode_ = mode;
                invalidate();
            }
        }

        auto setLineSpacing(float spacing) -> void
        {
            if (lineSpacing_ != spacing)
            {
                lineSpacing_ = spacing;
                invalidate();
            }
        }

        [[nodiscard]] auto measure(Size availableSize) -> Size override
        {
            relayoutIfNeeded(availableSize.width);
            return Size{.width = measuredWidth_, .height = measuredHeight_};
        }

        auto arrange(Rect bounds) -> void override
        {
            Widget::arrange(bounds);
            relayoutIfNeeded(bounds.width);
            rebuildChildren();
        }

    private:
        struct Fragment
        {
            std::string text;
            awen::sdl::FontHandle font{};
            int fontSize{};
            Color color{};
            float x{};
            float y{};
        };

        struct Token
        {
            std::string_view text;
            std::size_t runIndex{};
            float width{};
            bool isWhitespace{};
            bool isNewline{};
        };

        auto invalidate() -> void
        {
            layoutValid_ = false;
        }

        auto relayoutIfNeeded(float width) -> void
        {
            if (layoutValid_ && hasUsableLayout_ && lastLayoutWidth_ == width)
            {
                return;
            }

            layout(width);
            lastLayoutWidth_ = width;
            layoutValid_ = hasUsableLayout_;
        }

        [[nodiscard]] static auto measureToken(const TextRun& run, std::string_view text) -> float
        {
            if (std::empty(text))
            {
                return 0.0F;
            }

            const auto copy = std::string{text};
            const auto width = awen::sdl::Renderer::measureText(std::optional{run.font}, copy.c_str(), run.fontSize);
            return static_cast<float>(width.value_or(0));
        }

        [[nodiscard]] auto tokenize() const -> std::vector<Token>
        {
            auto tokens = std::vector<Token>{};

            for (auto i = std::size_t{0}; i < std::size(runs_); ++i)
            {
                const auto& run = runs_[i];
                const auto sv = std::string_view{run.text};
                auto pos = std::size_t{0};

                while (pos < std::size(sv))
                {
                    if (sv[pos] == '\n')
                    {
                        tokens.push_back(Token{.text = "", .runIndex = i, .isNewline = true});
                        ++pos;
                        continue;
                    }

                    if (sv[pos] == ' ' || sv[pos] == '\t')
                    {
                        auto end = pos;

                        while (end < std::size(sv) && (sv[end] == ' ' || sv[end] == '\t'))
                        {
                            ++end;
                        }

                        const auto text = sv.substr(pos, end - pos);
                        tokens.push_back(Token{
                            .text = text,
                            .runIndex = i,
                            .width = measureToken(run, text),
                            .isWhitespace = true,
                        });
                        pos = end;
                        continue;
                    }

                    auto end = pos;

                    while (end < std::size(sv) && sv[end] != ' ' && sv[end] != '\t' && sv[end] != '\n')
                    {
                        ++end;
                    }

                    const auto text = sv.substr(pos, end - pos);
                    tokens.push_back(Token{
                        .text = text,
                        .runIndex = i,
                        .width = measureToken(run, text),
                    });
                    pos = end;
                }
            }

            return tokens;
        }

        auto buildLineFragments(const std::vector<Token>& lineTokens) -> void
        {
            auto lineHeight = 0.0F;

            for (const auto& tok : lineTokens)
            {
                lineHeight = std::max(lineHeight, static_cast<float>(runs_[tok.runIndex].fontSize));
            }

            if (lineHeight <= 0.0F && !std::empty(runs_))
            {
                lineHeight = static_cast<float>(runs_.front().fontSize);
            }

            auto x = 0.0F;
            auto lineWidth = 0.0F;

            for (const auto& tok : lineTokens)
            {
                if (tok.isWhitespace)
                {
                    x += tok.width;
                    continue;
                }

                const auto& run = runs_[tok.runIndex];
                fragments_.push_back(Fragment{
                    .text = std::string{tok.text},
                    .font = run.font,
                    .fontSize = run.fontSize,
                    .color = run.color,
                    .x = x,
                    .y = measuredHeight_,
                });
                x += tok.width;
                lineWidth = x;
            }

            measuredWidth_ = std::max(measuredWidth_, lineWidth);
            measuredHeight_ += lineHeight + lineSpacing_;
        }

        auto layout(float maxWidth) -> void
        {
            fragments_.clear();
            measuredWidth_ = 0.0F;
            measuredHeight_ = 0.0F;

            if (std::empty(runs_))
            {
                return;
            }

            const auto tokens = tokenize();

            // Detect the case where text has been provided but the renderer
            // could not measure any of it (e.g. fonts not yet loaded during
            // a pre-startup arrange).  Bail without caching so the next
            // arrange call retries once measurement is available.
            auto anyMeasurable = false;

            for (const auto& tok : tokens)
            {
                if (!tok.isWhitespace && !tok.isNewline && !std::empty(tok.text) && tok.width > 0.0F)
                {
                    anyMeasurable = true;
                    break;
                }
            }

            if (!anyMeasurable)
            {
                hasUsableLayout_ = false;
                return;
            }

            hasUsableLayout_ = true;
            const auto wrap = wrapMode_ == WrapMode::Word && maxWidth > 0.0F;

            auto lineX = 0.0F;
            auto lineTokens = std::vector<Token>{};

            const auto flushLine = [&]
            {
                buildLineFragments(lineTokens);
                lineTokens.clear();
                lineX = 0.0F;
            };

            for (const auto& tok : tokens)
            {
                if (tok.isNewline)
                {
                    flushLine();
                    continue;
                }

                const auto next = lineX + tok.width;

                if (wrap && !tok.isWhitespace && next > maxWidth && !std::empty(lineTokens))
                {
                    while (!std::empty(lineTokens) && lineTokens.back().isWhitespace)
                    {
                        lineX -= lineTokens.back().width;
                        lineTokens.pop_back();
                    }

                    flushLine();
                }

                if (tok.isWhitespace && std::empty(lineTokens))
                {
                    continue;
                }

                lineTokens.push_back(tok);
                lineX += tok.width;
            }

            flushLine();
        }

        auto rebuildChildren() -> void
        {
            // Drop existing fragment children, then recreate from `fragments_`.
            // We track only children we created so other Object children
            // remain untouched.
            for (auto* child : ownedChildren_)
            {
                std::ignore = child->remove();
            }

            ownedChildren_.clear();

            for (const auto& frag : fragments_)
            {
                auto transform = std::make_unique<NodeTransform>();
                transform->setPosition(glm::vec2{frag.x, frag.y});

                auto text = std::make_unique<NodeText>();
                text->setText(frag.text);
                text->setFontSize(frag.fontSize);
                text->setColor(frag.color);
                text->setFont(frag.font);
                transform->addChild(std::move(text));

                auto* raw = transform.get();
                ownedChildren_.push_back(raw);
                this->addChild(std::move(transform));
            }
        }

        std::vector<TextRun> runs_;
        std::vector<Fragment> fragments_;
        std::vector<awen::core::Object*> ownedChildren_;
        WrapMode wrapMode_{WrapMode::None};
        float lineSpacing_{4.0F};
        float measuredWidth_{};
        float measuredHeight_{};
        float lastLayoutWidth_{-1.0F};
        bool layoutValid_{false};
        bool hasUsableLayout_{false};
    };
}
