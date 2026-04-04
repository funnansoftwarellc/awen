#include <gtest/gtest.h>
#include <vector>

import awen.core.signal;

TEST(Signal, emit_with_no_connections)
{
    awn::core::Signal<void(int)> sig;

    // Should not crash with no connected slots.
    sig.emit(42);
}

TEST(Signal, connect_and_emit)
{
    awn::core::Signal<void(int)> sig;
    auto received = int{};

    sig.connect([&](int v) { received = v; });
    sig.emit(7);

    EXPECT_EQ(received, 7);
}

TEST(Signal, zero_arg_signal)
{
    awn::core::Signal<void()> sig;
    auto count = int{};

    sig.connect([&] { ++count; });
    sig.emit();
    sig.emit();

    EXPECT_EQ(count, 2);
}

TEST(Signal, multiple_slots_invoked_in_order)
{
    awn::core::Signal<void()> sig;
    auto order = std::vector<int>{};

    sig.connect([&] { order.push_back(1); });
    sig.connect([&] { order.push_back(2); });
    sig.connect([&] { order.push_back(3); });
    sig.emit();

    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

TEST(Signal, manual_disconnect)
{
    awn::core::Signal<void(int)> sig;
    auto count = int{};

    auto conn = sig.connect([&](int) { ++count; });
    sig.emit(0);

    conn.disconnect();
    sig.emit(0);

    EXPECT_EQ(count, 1);
}

TEST(Signal, disconnect_is_idempotent)
{
    awn::core::Signal<void()> sig;
    auto count = int{};

    auto conn = sig.connect([&] { ++count; });
    conn.disconnect();
    conn.disconnect();
    sig.emit();

    EXPECT_EQ(count, 0);
}

TEST(Signal, connection_connected_reflects_state)
{
    awn::core::Signal<void()> sig;
    auto conn = sig.connect([] {});

    EXPECT_TRUE(conn.connected());
    conn.disconnect();
    EXPECT_FALSE(conn.connected());
}

TEST(Signal, scoped_connection_disconnects_on_scope_exit)
{
    awn::core::Signal<void(int)> sig;
    auto count = int{};

    {
        auto sc = awn::core::scoped_connection{sig.connect([&](int) { ++count; })};
        sig.emit(0);
        EXPECT_EQ(count, 1);
    }

    sig.emit(0);
    EXPECT_EQ(count, 1);
}

TEST(Signal, scoped_connection_connected_reflects_state)
{
    awn::core::Signal<void()> sig;
    auto sc = awn::core::scoped_connection{sig.connect([] {})};

    EXPECT_TRUE(sc.connected());
    sc.disconnect();
    EXPECT_FALSE(sc.connected());
}

TEST(Signal, signal_destroyed_while_connection_alive)
{
    auto conn = awn::core::Connection{};

    {
        awn::core::Signal<void(int)> sig;
        conn = sig.connect([](int) {});
        EXPECT_TRUE(conn.connected());
    }

    // Signal is gone — connected() must return false and disconnect() must not crash.
    EXPECT_FALSE(conn.connected());
    conn.disconnect();
}

TEST(Signal, scoped_connection_move_assignment_disconnects_old_slot)
{
    awn::core::Signal<void()> sig;
    auto count_a = int{};
    auto count_b = int{};

    auto sc = awn::core::scoped_connection{sig.connect([&] { ++count_a; })};
    sig.emit();
    EXPECT_EQ(count_a, 1);

    // Move-assign a new connection — the old slot must be disconnected.
    sc = awn::core::scoped_connection{sig.connect([&] { ++count_b; })};
    sig.emit();

    EXPECT_EQ(count_a, 1);
    EXPECT_EQ(count_b, 1);
}

TEST(Signal, scoped_connection_release_keeps_slot_alive)
{
    awn::core::Signal<void()> sig;
    auto count = int{};
    auto conn = awn::core::Connection{};

    {
        auto sc = awn::core::scoped_connection{sig.connect([&] { ++count; })};

        // Release transfers ownership — sc's destructor must NOT disconnect.
        conn = sc.release();
    }

    sig.emit();
    EXPECT_EQ(count, 1);

    conn.disconnect();
    sig.emit();
    EXPECT_EQ(count, 1);
}

TEST(Signal, slot_disconnects_self_during_emit)
{
    awn::core::Signal<void()> sig;
    auto count = int{};
    auto conn = awn::core::Connection{};

    conn = sig.connect(
        [&]
        {
            ++count;
            conn.disconnect();
        });

    sig.emit();
    sig.emit();

    // The self-disconnecting slot must fire only once.
    EXPECT_EQ(count, 1);
}

TEST(Signal, remaining_slots_fire_after_peer_disconnects_during_emit)
{
    awn::core::Signal<void()> sig;
    auto count_a = int{};
    auto count_b = int{};
    auto conn_a = awn::core::Connection{};

    conn_a = sig.connect(
        [&]
        {
            ++count_a;
            conn_a.disconnect();
        });

    sig.connect([&] { ++count_b; });

    sig.emit();

    EXPECT_EQ(count_a, 1);
    EXPECT_EQ(count_b, 1);

    sig.emit();

    EXPECT_EQ(count_a, 1);
    EXPECT_EQ(count_b, 2);
}
