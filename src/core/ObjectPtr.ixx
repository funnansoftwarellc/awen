module;

#include <compare>
#include <memory>

export module awen.core.objectptr;
import awen.core.object;

export namespace awen::core
{
    template <typename T>
    class ObjectPtr
    {
    public:
        ObjectPtr() = default;

        explicit ObjectPtr(T* x) : ptr_(x)
        {
            if (ptr_)
            {
                ptr_->on_destroyed.connect([this] { ptr_ = nullptr; });
            }
        }

        explicit ObjectPtr(const std::unique_ptr<T>& x) : ObjectPtr(x.get())
        {
        }

        [[nodiscard]] auto operator->() const -> T*
        {
            return ptr_;
        }

        [[nodiscard]] auto operator*() const -> T&
        {
            return *ptr_;
        }

        auto operator<=>(const ObjectPtr&) const = default;

        auto operator==(const ObjectPtr&) const -> bool = default;

        auto operator==(std::nullptr_t) const -> bool
        {
            return ptr_ == nullptr;
        }

    private:
        T* ptr_ = nullptr;
    };
}