module;

#include <sigslot/signal.hpp>

export module awen.core.object;

export namespace awen::core
{
    class Object
    {
    public:
        Object() = default;
        Object(const Object&) = delete;
        Object(Object&&) noexcept = delete;
        auto operator=(const Object&) -> Object& = delete;
        auto operator=(Object&&) noexcept -> Object& = delete;

        virtual ~Object()
        {
            onDestroyed();
        }

        sigslot::signal<> onDestroyed;
    };
}