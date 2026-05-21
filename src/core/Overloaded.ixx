export module awen.core.overloaded;

export namespace awen::core
{
    template <typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };

    // template <typename... Ts>
    // Overloaded(Ts...) -> Overloaded<Ts...>;
}