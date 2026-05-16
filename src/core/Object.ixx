module;

#include <concepts>
#include <memory>
#include <sigslot/signal.hpp>
#include <vector>

export module awen.core.object;

export namespace awen::core
{
    class Object;

    template <typename T>
    concept TypeObject = std::derived_from<T, Object>;

    class Object
    {
    public:
        Object() = default;
        Object(const Object&) = delete;
        auto operator=(const Object&) -> Object& = delete;
        Object(Object&&) noexcept = delete;
        auto operator=(Object&&) noexcept -> Object& = delete;

        virtual ~Object()
        {
            onDestroyed();
        }

        auto addChild(std::unique_ptr<Object> x) -> Object*
        {
            if (!x)
            {
                return nullptr;
            }

            auto* childPtr = x.get();
            children_.emplace_back(std::move(x));
            return childPtr;
        }

        template <TypeObject T, typename... Args>
        auto addChild(Args&&... args) -> T*
        {
            auto child = std::make_unique<T>(std::forward<Args>(args)...);
            auto* childPtr = child.get();
            children_.emplace_back(std::move(child));
            return childPtr;
        }

        sigslot::signal<> onDestroyed;

    private:
        std::vector<std::unique_ptr<Object>> children_;
    };
}