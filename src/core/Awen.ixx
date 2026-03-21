module;

export module awen;

export namespace awn
{
    class Awen
    {
    public:
        auto set_value(int x) noexcept -> void
        {
            value_ = x;
        }

        [[nodiscard]] auto get_value() const noexcept -> int
        {
            return value_;
        }

    private:
        int value_;
    };
}