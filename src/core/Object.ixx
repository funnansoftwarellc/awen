module;

#include <compare>
#include <memory>
#include <ranges>
#include <string_view>
#include <vector>

export module awen.core.object;
export import awen.core.signal;

export namespace awen::core
{
    /// @brief Base class for all objects in the engine.
    class Object
    {
    public:
        Object() = default;

        virtual ~Object()
        {
            on_destroyed.emit();
        }

        Object(const Object&) = delete;
        auto operator=(const Object&) -> Object& = delete;
        Object(Object&&) noexcept = delete;
        auto operator=(Object&&) noexcept -> Object& = delete;

        /// @brief Adds a child object to this object.
        /// @param child The child object to add.
        auto set_name(std::string_view x) -> void
        {
            name_ = x;
        }

        /// @brief Gets the name of this object.
        /// @return The name of this object.
        [[nodiscard]] auto get_name() const -> std::string_view
        {
            return name_;
        }

        /// @brief Adds a child object to this object.
        /// @param child The child object to add.
        auto add_child(std::unique_ptr<Object> x) -> void
        {
            if (x == nullptr)
            {
                return;
            }

            x->parent_ = this;
            children_.emplace_back(std::move(x));

            // If this object has already started, immediately notify the new child.
            if (started_)
            {
                children_.back()->startup();
            }
        }

        /// @brief Removes this object from its parent.
        auto remove() -> std::unique_ptr<Object>
        {
            if (parent_ == nullptr)
            {
                return nullptr;
            }

            const auto it = std::ranges::find_if(parent_->children_, [this](const auto& x) { return x.get() == this; });

            if (it == std::end(parent_->children_))
            {
                return nullptr;
            }

            auto result = std::move(*it);
            parent_->children_.erase(it);
            parent_ = nullptr;
            return result;
        }

        /// @brief Get the children of this object.
        /// @return The children of this object.
        [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<Object>>&
        {
            return children_;
        }

        /// @brief Gets the parent object of this object.
        /// @return The parent object of this object, or nullptr if this object has no parent.
        [[nodiscard]] auto get_parent() const -> Object*
        {
            return parent_;
        }

        auto startup() -> void
        {
            if (started_)
            {
                return;
            }

            started_ = true;

            on_startup.emit();

            for (const auto& child : children_)
            {
                child->startup();
            }
        }

        Signal<void()> on_destroyed;
        Signal<void()> on_startup;

    private:
        std::vector<std::unique_ptr<Object>> children_;
        std::string_view name_;
        Object* parent_ = nullptr;
        bool started_ = false;
    };
}