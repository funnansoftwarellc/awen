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
        enum class FindOption : std::uint8_t
        {
            Direct,
            Recursive,
        };

        Object() = default;
        Object(const Object&) = delete;
        auto operator=(const Object&) -> Object& = delete;
        Object(Object&&) noexcept = delete;
        auto operator=(Object&&) noexcept -> Object& = delete;

        virtual ~Object()
        {
            destroyed_();
        }

        auto startup() -> void
        {
            if (started_)
            {
                return;
            }

            started_ = true;

            startup_();

            for (const auto& child : children_)
            {
                child->startup();
            }
        }

        auto updatePre() -> void
        {
            updatePre_();

            for (const auto& child : children_)
            {
                child->updatePre();
            }
        }

        /// @brief Updates this object and all its children. The update is performed by emitting the update signal, which can be connected to by any
        /// slot. The update signal is emitted with a duration parameter that represents the time elapsed since the last update. The update function
        /// also recursively calls the update function of all its children, passing the same duration parameter.
        /// @param x The duration parameter that represents the time elapsed since the last update. This parameter is passed to the update signal and
        /// to the update function of all children.
        auto update(std::chrono::duration<float> x) -> void
        {
            update_(x);

            for (const auto& child : children_)
            {
                child->update(x);
            }
        }

        auto updatePost() -> void
        {
            updatePost_();

            for (const auto& child : children_)
            {
                child->updatePost();
            }
        }

        /// @brief Performs fixed updates on this object and all its children. The fixed update is performed by emitting the updateFixed signal, which
        /// can be connected to by any slot. The updateFixed signal is emitted with a duration parameter that represents the time elapsed since the
        /// last fixed update. The updateFixed function also recursively calls the updateFixed function of all its children, passing the same duration
        /// parameter.
        /// @param x The duration parameter that represents the time elapsed since the last fixed update.
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
            childPtr->clearParentCache();
            childAdd_(*childPtr);
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
            return static_cast<T*>(addChild(std::move(child)));
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

            auto* oldParent = parent_;
            auto removedChild = oldParent->removeChild(this);

            if (removedChild == nullptr)
            {
                return nullptr;
            }

            removedChild->parent_ = nullptr;
            removedChild->clearParentCache();
            oldParent->childRemove_(*removedChild);
            return removedChild;
        }

        /// @brief Get the child objects of this object.
        /// @return The vector of unique pointers to the child objects.
        auto getChildren(FindOption option = FindOption::Direct) const -> std::vector<Object*>
        {
            std::vector<Object*> v;
            v.reserve(std::size(children_));

            switch (option)
            {
                case FindOption::Recursive:
                {
                    std::vector<Object*> visitor;
                    visitor.reserve(std::size(children_));

                    for (const auto& child : std::views::reverse(children_))
                    {
                        visitor.emplace_back(child.get());
                    }

                    while (!std::empty(visitor))
                    {
                        // Process the first child/parent.
                        auto* parent = visitor.back();
                        visitor.pop_back();
                        v.emplace_back(parent);

                        for (const auto& child : std::views::reverse(parent->children_))
                        {
                            visitor.emplace_back(child.get());
                        }
                    }
                }
                break;

                case FindOption::Direct:
                    [[fallthrough]];
                default:
                    for (const auto& child : children_)
                    {
                        v.emplace_back(child.get());
                    }
                    break;
            }

            return v;
        }

        /// @brief Get the child objects of this object that satisfy a given predicate.
        /// @param x The predicate to filter the child objects. Must be a callable that takes a pointer to an Object and returns a boolean.
        /// @param option The option to specify whether to search recursively or not.
        /// @return The vector of pointers to the child objects that satisfy the predicate.
        auto getChildren(auto x, FindOption option = FindOption::Direct) const -> std::vector<Object*>
        {
            auto temp = getChildren(option);
            std::vector<Object*> v;
            std::ranges::copy_if(temp, std::back_inserter(v), x);
            return v;
        }

        /// @brief Gets the parent object of this object.
        /// @return The pointer to the parent object, or nullptr if this object has no parent.
        auto getParent() const -> Object*
        {
            return parent_;
        }

        template <TypeObject T>
        [[nodiscard]] auto getParent() const -> T*
        {
            if (parent_ == nullptr)
            {
                return nullptr;
            }

            auto* cached = static_cast<T*>(parentCache_[std::type_index(typeid(T))]);

            if (cached == nullptr)
            {
                if (auto* parentT = dynamic_cast<T*>(parent_))
                {
                    cached = parentT;
                }
                else
                {
                    cached = parent_->getParent<T>();
                }

                parentCache_[std::type_index(typeid(T))] = cached;
            }

            return cached;
        }

        auto onDestroyed(auto x) -> sigslot::connection
        {
            return destroyed_.connect(x);
        }

        auto onStartup(auto x) -> sigslot::connection
        {
            return startup_.connect(x);
        }

        auto onUpdatePre(auto x) -> sigslot::connection
        {
            return updatePre_.connect(x);
        }

        auto onUpdate(auto x) -> sigslot::connection
        {
            return update_.connect(x);
        }

        auto onUpdatePost(auto x) -> sigslot::connection
        {
            return updatePost_.connect(x);
        }

        auto onUpdateFixed(auto x) -> sigslot::connection
        {
            return updateFixed_.connect(x);
        }

        /// @brief Connects a slot that is called after a child object is added to this object.
        /// @param x The slot to connect. The slot receives the added child object.
        /// @return The connection that can be used to disconnect the slot.
        auto onChildAdd(auto x) -> sigslot::connection
        {
            return childAdd_.connect(x);
        }

        /// @brief Connects a slot that is called after a child object is removed from this object.
        /// @param x The slot to connect. The slot receives the removed child object.
        /// @return The connection that can be used to disconnect the slot.
        auto onChildRemove(auto x) -> sigslot::connection
        {
            return childRemove_.connect(x);
        }

    private:
        auto clearParentCache() -> void
        {
            std::vector<Object*> visitor;
            std::vector<Object*> visited;
            visitor.reserve(std::size(children_));
            visited.reserve(std::size(children_) + 1);
            visitor.emplace_back(this);

            while (!std::empty(visitor))
            {
                auto* object = visitor.back();
                visitor.pop_back();

                if (std::ranges::find(visited, object) != std::end(visited))
                {
                    continue;
                }

                visited.emplace_back(object);
                object->parentCache_.clear();

                for (const auto& child : object->children_)
                {
                    visitor.emplace_back(child.get());
                }
            }
        }

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

        std::vector<std::unique_ptr<Object>> children_;
        mutable std::unordered_map<std::type_index, Object*> parentCache_;
        Object* parent_{};
        sigslot::signal_st<> destroyed_;
        sigslot::signal_st<> startup_;
        sigslot::signal_st<> updatePre_;
        sigslot::signal_st<std::chrono::duration<float>> update_;
        sigslot::signal_st<std::chrono::duration<float>> updateFixed_;
        sigslot::signal_st<> updatePost_;
        sigslot::signal_st<Object&> childAdd_;
        sigslot::signal_st<Object&> childRemove_;
        bool started_{false};
    };
}