module;

#include <compare>
#include <exception>
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

        virtual ~Object() noexcept
        {
            try
            {
                onDestroyed_.emit();
            }
            catch (...)
            {
                const auto exception = std::current_exception();
                static_cast<void>(exception);
            }
        }

        Object(const Object&) = delete;
        auto operator=(const Object&) -> Object& = delete;
        Object(Object&&) noexcept = delete;
        auto operator=(Object&&) noexcept -> Object& = delete;

        /// @brief Adds a child object to this object.
        /// @param child The child object to add.
        auto setName(std::string_view x) -> void
        {
            name_ = x;
        }

        /// @brief Gets the name of this object.
        /// @return The name of this object.
        [[nodiscard]] auto getName() const -> std::string_view
        {
            return name_;
        }

        /// @brief Adds a child object to this object.
        /// @param child The child object to add.
        auto addChild(std::unique_ptr<Object> x) -> void
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

            auto it = std::end(parent_->children_);

            for (auto childIt = std::begin(parent_->children_); childIt != std::end(parent_->children_); ++childIt)
            {
                if (childIt->get() == this)
                {
                    it = childIt;
                    break;
                }
            }

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
        [[nodiscard]] auto getChildren() const -> const std::vector<std::unique_ptr<Object>>&
        {
            return children_;
        }

        /// @brief Gets the parent object of this object.
        /// @return The parent object of this object, or nullptr if this object has no parent.
        [[nodiscard]] auto getParent() const -> Object*
        {
            return parent_;
        }

        auto startup() -> void
        {
            if (started_)
            {
                return;
            }

            auto stack = std::vector<Object*>{this};

            while (!std::empty(stack))
            {
                auto* object = stack.back();
                stack.pop_back();

                if (object->started_)
                {
                    continue;
                }

                object->started_ = true;
                object->onStartup_.emit();

                for (auto& child : object->children_ | std::views::reverse)
                {
                    stack.push_back(child.get());
                }
            }
        }

        [[nodiscard]] auto onDestroyed() -> Signal<void()>&
        {
            return onDestroyed_;
        }

        [[nodiscard]] auto onStartup() -> Signal<void()>&
        {
            return onStartup_;
        }

    private:
        std::vector<std::unique_ptr<Object>> children_;
        std::string_view name_;
        Object* parent_ = nullptr;
        bool started_ = false;
        Signal<void()> onDestroyed_;
        Signal<void()> onStartup_;
    };
}