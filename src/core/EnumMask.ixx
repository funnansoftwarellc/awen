/// @file EnumMask.ixx
/// @module awen.core.enummask
module;

#include <initializer_list>
#include <type_traits>

export module awen.core.enummask;

export namespace awen::core
{
    template <typename T>
    concept TypeEnum = std::is_enum_v<T>;

    /// @class EnumMask
    /// @brief Type-safe bitmask utility for scoped enum flag types.
    ///
    /// `EnumMask` provides a lightweight, constexpr-friendly wrapper around
    /// bitmask-style enums (`enum class`) whose enumerators represent individual
    /// bit flags. It enforces type safety by requiring a `TypeEnum` and avoids
    /// implicit integer conversions.
    ///
    /// @tparam T Enum type used as the mask (must satisfy `TypeEnum`).
    ///
    /// @note Enum values of `T` are expected to be powers of two (bit flags).
    template <TypeEnum T>
    class EnumMask
    {
    public:
        /// @brief Underlying integer type of the enum.
        using UnderlyingType = std::underlying_type_t<T>;

        /// @brief Construct an empty mask with all bits cleared.
        constexpr EnumMask() = default;

        /// @brief Construct a mask initialized with the given enum values.
        ///
        /// Each enumerator in the initializer list is inserted into the mask
        /// by setting its corresponding bit.
        ///
        /// @param x Initial set of enum flags.
        constexpr explicit EnumMask(std::initializer_list<T> x)
        {
            for (auto e : x)
            {
                this->set(e);
            }
        }

        /// @brief Test whether a given enum flag is set.
        /// @param x Enum value to test.
        /// @return True if the flag is set, false otherwise.
        [[nodiscard]] constexpr auto test(T x) const noexcept -> bool
        {
            return (mask_ & static_cast<UnderlyingType>(x)) != 0x0;
        }

        /// @brief Set (enable) a given enum flag.
        /// @param x Enum value to set.
        constexpr auto set(T x) noexcept -> void
        {
            mask_ |= static_cast<UnderlyingType>(x);
        }

        /// @brief Get the raw underlying mask value.
        /// @return Underlying integer bitmask.
        [[nodiscard]] constexpr auto get() const noexcept -> UnderlyingType
        {
            return mask_;
        }

        /// @brief Remove (clear) a given enum flag.
        /// @param x Enum value to clear.
        constexpr auto remove(T x) noexcept -> void
        {
            mask_ &= ~static_cast<UnderlyingType>(x);
        }

        /// @brief Toggle (flip) a given enum flag.
        /// @param x Enum value to toggle.
        constexpr auto flip(T x) noexcept -> void
        {
            mask_ ^= static_cast<UnderlyingType>(x);
        }

        /// @brief Clear all flags from the mask.
        constexpr auto clear() noexcept -> void
        {
            mask_ = 0;
        }

    private:
        UnderlyingType mask_{};
    };
}