export module awen.overloaded;

export namespace awn
{
    /// @brief Combines multiple callable objects into a single overload set.
    ///
    /// Used primarily with std::visit to dispatch std::variant alternatives via a
    /// set of per-type lambdas instead of a single generic lambda with if constexpr.
    ///
    /// @tparam Ts The callable types to combine into the overload set.
    ///
    /// @note Relies on C++17 variadic using-declarations and class template argument
    ///       deduction — no explicit template arguments are needed at the call site.
    template <typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };
}
