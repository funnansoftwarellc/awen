module;

#include <algorithm>
#include <chrono>
#include <concepts>
#include <memory>
#include <ranges>
#include <sigslot/signal.hpp>
#include <typeindex>
#include <unordered_map>
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
            destroyed_();
        }

        /// @brief Updates this object and all its children. The update is performed by emitting the update signal, which can be connected to by any
        /// slot. The update signal is emitted with a duration parameter that represents the time elapsed since the last update. The update function
        /// also recursively calls the update function of all its children, passing the same duration parameter.
        /// @param x The duration parameter that represents the time elapsed since the last update. This parameter is passed to the update signal and
        /// to the update function of all children.
        // NOLINTNEXTLINE(misc-no-recursion)
        auto update(std::chrono::duration<float> x) -> void
        {
            update_(x);

            for (const auto& child : children_)
            {
                child->update(x);
            }
        }

        // NOLINTNEXTLINE(misc-no-recursion)
        auto updateFixed(std::chrono::duration<float> x) -> void
        {
            updateFixed_(x);

            for (const auto& child : children_)
            {
                child->updateFixed(x);
            }
        }

        /// @brief Adds a child object to this object. The child will be automatically destroyed when the parent is destroyed.
        /// @param x The child object to add. Must not be null.
        /// @return The pointer to the added child object, or nullptr if the input was null.
        auto addChild(std::unique_ptr<Object> x) -> Object*
        {
            if (!x)
            {
                return nullptr;
            }

            auto* childPtr = x.get();
            children_.emplace_back(std::move(x));
            childPtr->parent_ = this;
            typeChildren_[std::type_index(typeid(*childPtr))].emplace_back(childPtr);
            return childPtr;
        }

        /// @brief Adds a child object of type T to this object. The child will be automatically destroyed when the parent is destroyed.
        /// @tparam ...Args The types of the arguments to construct the child object. Must be compatible with the constructor of T.
        /// @tparam T The type of the child object to add. Must be derived from Object.
        /// @param ...args The arguments to construct the child object. Must be compatible with the constructor of T.
        /// @return The pointer to the added child object.
        template <TypeObject T, typename... Args>
        auto addChild(Args&&... args) -> T*
        {
            auto child = std::make_unique<T>(std::forward<Args>(args)...);
            auto* childPtr = child.get();
            children_.emplace_back(std::move(child));
            childPtr->parent_ = this;
            typeChildren_[std::type_index(typeid(*childPtr))].emplace_back(childPtr);
            return childPtr;
        }

        /// @brief Removes this object from its parent. The removed object will not be destroyed, but it will no longer be a child of the parent.
        /// @return The unique pointer to the removed object, or nullptr if this object has no parent or if the parent does not contain this object as
        /// a child.
        auto remove() -> std::unique_ptr<Object>
        {
            if (parent_ == nullptr)
            {
                return nullptr;
            }

            auto removeChild = parent_->removeChild(this);

            if (removeChild == nullptr)
            {
                return nullptr;
            }

            parent_->removeTypeChild(*removeChild);
            removeChild->parent_ = nullptr;
            return removeChild;
        }

        /// @brief Get the child objects of this object.
        /// @return The vector of unique pointers to the child objects.
        auto getChildren() const -> const std::vector<std::unique_ptr<Object>>&
        {
            return children_;
        }

        /// @brief Get the child objects of this object that satisfy a given predicate.
        /// @param x The predicate to filter the child objects. Must be a callable that takes a pointer to an Object and returns a boolean.
        /// @return The vector of pointers to the child objects that satisfy the predicate.
        auto getChildren(auto x) const -> std::vector<Object*>
        {
            std::vector<Object*> children;
            children.reserve(children_.size());

            for (const auto& child : children_)
            {
                if (x(child.get()))
                {
                    children.emplace_back(child.get());
                }
            }

            return children;
        }

        template <TypeObject T>
        auto getChildren() const -> std::vector<T*>
        {
            std::vector<T*> children;

            auto it = typeChildren_.find(std::type_index(typeid(T)));

            if (it == std::end(typeChildren_))
            {
                return children;
            }

            std::ranges::transform(it->second, std::back_inserter(children), [](auto* obj) { return static_cast<T*>(obj); });

            return children;
        }

        /// @brief Gets the parent object of this object.
        /// @return The pointer to the parent object, or nullptr if this object has no parent.
        auto getParent() const -> Object*
        {
            return parent_;
        }

        auto onDestroyed(auto x) -> sigslot::connection
        {
            return destroyed_.connect(x);
        }

        auto onUpdate(auto x) -> sigslot::connection
        {
            return update_.connect(x);
        }

        auto onUpdateFixed(auto x) -> sigslot::connection
        {
            return updateFixed_.connect(x);
        }

    private:
        auto removeChild(Object* child) -> std::unique_ptr<Object>
        {
            auto it = std::ranges::find_if(children_, [child](const auto& c) { return c.get() == child; });

            if (it == std::end(children_))
            {
                return nullptr;
            }

            auto removedChild = std::move(*it);
            children_.erase(it);
            return removedChild;
        }

        auto removeTypeChild(const Object& child) -> void
        {
            for (auto& [type, children] : typeChildren_)
            {
                auto it = std::ranges::find(children, &child);
                if (it != std::end(children))
                {
                    children.erase(it);
                    break;
                }
            }
        }

        std::vector<std::unique_ptr<Object>> children_;
        std::unordered_map<std::type_index, std::vector<Object*>> typeChildren_;
        Object* parent_{};
        sigslot::signal<> destroyed_;
        sigslot::signal<std::chrono::duration<float>> update_;
        sigslot::signal<std::chrono::duration<float>> updateFixed_;
    };
}