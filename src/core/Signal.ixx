module;

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

export module awen.signal;

// Internal implementation — not exported to module consumers.
namespace awn::detail
{

    /// @brief Shared control block for a Signal. Owned by the Signal via shared_ptr;
    ///        Connection handles hold a weak_ptr to detect signal destruction.
    template <typename... Args>
    class SignalCore : public std::enable_shared_from_this<SignalCore<Args...>>
    {
    public:
        using SlotFn = std::function<void(Args...)>;

        /// @brief Registers a slot and returns the liveness token and a type-erased disconnect callable.
        auto connect(SlotFn fn) -> std::pair<std::weak_ptr<bool>, std::function<void()>>
        {
            auto alive = std::make_shared<bool>(true);
            const auto id = next_id_++;
            slots_.push_back(SlotEntry{.id = id, .fn = std::move(fn), .alive = alive});

            auto weak_core = this->weak_from_this();

            return {
                std::weak_ptr<bool>(alive),
                [weak_core, id]()
                {
                    if (auto core = weak_core.lock())
                    {
                        core->do_disconnect(id);
                    }
                },
            };
        }

        /// @brief Invokes all live slots. Defers slot erasure until after the outermost emit returns.
        auto emit(Args... args) -> void
        {
            ++emit_depth_;

            for (auto& entry : slots_)
            {
                if (*entry.alive)
                {
                    entry.fn(args...);
                }
            }

            --emit_depth_;

            if (emit_depth_ == 0)
            {
                std::erase_if(slots_, [](const SlotEntry& e) { return !*e.alive; });
            }
        }

    private:
        struct SlotEntry
        {
            uint64_t id{};
            SlotFn fn;
            std::shared_ptr<bool> alive;
        };

        /// @brief Marks a slot dead by ID and prunes the list when not emitting.
        auto do_disconnect(const uint64_t id) -> void
        {
            for (auto& entry : slots_)
            {
                if (entry.id == id)
                {
                    *entry.alive = false;
                    break;
                }
            }

            if (emit_depth_ == 0)
            {
                std::erase_if(slots_, [](const SlotEntry& e) { return !*e.alive; });
            }
        }

        std::vector<SlotEntry> slots_;
        uint64_t next_id_{};
        int emit_depth_{};
    };

} // namespace awn::detail

export namespace awn
{

    /// @brief Non-owning handle to a signal-slot connection.
    ///
    /// Safe to hold after the originating Signal is destroyed — disconnect() and
    /// connected() become no-ops once the signal's control block is gone.
    class Connection
    {
    public:
        /// @brief Constructs a null, disconnected connection.
        Connection() = default;

        /// @brief Disconnects the slot from its signal. Idempotent.
        auto disconnect() -> void
        {
            if (disconnect_fn_)
            {
                disconnect_fn_();
                disconnect_fn_ = nullptr;
            }
        }

        /// @brief Returns true if the slot is connected and the signal is still alive.
        [[nodiscard]] auto connected() const noexcept -> bool
        {
            const auto ptr = alive_.lock();
            return ptr && *ptr;
        }

    private:
        template <typename>
        friend class Signal;

        Connection(std::weak_ptr<bool> alive, std::function<void()> fn) : alive_(std::move(alive)), disconnect_fn_(std::move(fn))
        {
        }

        std::weak_ptr<bool> alive_;
        std::function<void()> disconnect_fn_;
    };

    /// @brief RAII handle that automatically disconnects its slot when destroyed.
    ///
    /// Move-only. Move-assigning a new scoped_connection first disconnects any
    /// previously held slot. Use release() to surrender ownership without disconnecting.
    class scoped_connection
    {
    public:
        /// @brief Constructs an empty, disconnected scoped_connection.
        scoped_connection() = default;

        /// @brief Takes ownership of @p conn, disconnecting it on destruction.
        explicit scoped_connection(Connection conn) : connection_(std::move(conn))
        {
        }

        ~scoped_connection()
        {
            connection_.disconnect();
        }

        scoped_connection(const scoped_connection&) = delete;
        auto operator=(const scoped_connection&) -> scoped_connection& = delete;

        scoped_connection(scoped_connection&&) = default;

        /// @brief Disconnects the currently held slot and takes ownership of @p other's slot.
        auto operator=(scoped_connection&& other) noexcept -> scoped_connection&
        {
            if (this != &other)
            {
                connection_.disconnect();
                connection_ = std::move(other.connection_);
            }

            return *this;
        }

        /// @brief Disconnects the slot immediately. Idempotent.
        auto disconnect() -> void
        {
            connection_.disconnect();
        }

        /// @brief Returns true if the slot is still connected.
        [[nodiscard]] auto connected() const noexcept -> bool
        {
            return connection_.connected();
        }

        /// @brief Transfers ownership of the connection without disconnecting it.
        ///
        /// This scoped_connection becomes empty after the call.
        /// @return The underlying Connection.
        [[nodiscard]] auto release() -> Connection
        {
            return std::exchange(connection_, Connection{});
        }

    private:
        Connection connection_;
    };

    /// @brief Constrains a callable to be invocable with the given argument types.
    ///
    /// @tparam F  The callable type.
    /// @tparam Args  The argument types it must accept.
    template <typename F, typename... Args>
    concept SlotCallable = std::invocable<F, Args...>;

    /// @brief Signal that broadcasts to all connected slots when emitted.
    ///
    /// Declared using function-signature syntax: @c Signal<R(Args...)>.
    /// The return type @p R must be @c void; it is accepted for consistency with
    /// the @c std::function declaration style (e.g. @c Signal<void()>).
    ///
    /// @note Only the @c R(Args...) partial specialisation is defined.
    template <typename Signature>
    class Signal;

    /// @brief Partial specialisation of Signal for function-signature syntax.
    ///
    /// Non-copyable but movable. Existing Connection handles remain valid after a
    /// move. Slots are invoked in connection order. Slots may safely disconnect
    /// themselves or other slots during emit.
    ///
    /// @tparam R     Return type (must be @c void; declared for consistency with function-signature syntax).
    /// @tparam Args  Argument types forwarded to each slot on emit.
    template <typename R, typename... Args>
    class Signal<R(Args...)>
    {
    public:
        Signal() : core_(std::make_shared<detail::SignalCore<Args...>>())
        {
        }

        Signal(const Signal&) = delete;
        auto operator=(const Signal&) -> Signal& = delete;

        Signal(Signal&&) = default;
        auto operator=(Signal&&) -> Signal& = default;

        /// @brief Connects a callable slot to this signal.
        ///
        /// @tparam F  A callable satisfying SlotCallable<F, Args...>.
        /// @param fn  The slot to connect.
        /// @return A Connection handle for managing the slot's lifetime.
        template <typename F>
            requires SlotCallable<F, Args...>
        auto connect(F&& fn) -> Connection
        {
            auto [alive, disconnect_fn] = core_->connect(std::function<void(Args...)>(std::forward<F>(fn)));

            return Connection{std::move(alive), std::move(disconnect_fn)};
        }

        /// @brief Emits the signal, invoking all connected slots with @p args.
        ///
        /// @param args  Arguments forwarded to each slot.
        auto emit(Args... args) -> void
        {
            core_->emit(std::forward<Args>(args)...);
        }

    private:
        std::shared_ptr<detail::SignalCore<Args...>> core_;
    };

} // namespace awn
