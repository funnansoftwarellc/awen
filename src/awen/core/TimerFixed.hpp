#pragma once

#include <awen/core/export.hxx>

#include <QObject>
#include <QTimer>
#include <QtQml>
#include <chrono>

namespace awen::core
{
    /// @brief A timer that emits a signal at fixed intervals, regardless of the execution time of the slot.
    class AWEN_CORE_EXPORT TimerFixed : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(int interval READ getInterval WRITE setInterval NOTIFY intervalChanged)
        Q_PROPERTY(bool running READ getRunning WRITE setRunning NOTIFY runningChanged)
        Q_PROPERTY(bool repeat READ getRepeat WRITE setRepeat NOTIFY repeatChanged)
        QML_ELEMENT

    public:
        explicit TimerFixed(QObject* parent = nullptr);
        ~TimerFixed() override = default;

        TimerFixed(const TimerFixed&) = delete;
        auto operator=(const TimerFixed&) -> TimerFixed& = delete;
        TimerFixed(TimerFixed&&) noexcept = delete;
        auto operator=(TimerFixed&&) noexcept -> TimerFixed& = delete;

        /// @brief Sets the interval of the timer in milliseconds. The timer will emit the triggered signal every interval milliseconds, regardless of
        /// the execution time of the slot.
        /// @param x The interval in milliseconds.
        void setInterval(int x) noexcept;

        /// @brief Gets the interval of the timer in milliseconds.
        /// @return The interval in milliseconds.
        [[nodiscard]] int getInterval() const noexcept;

        /// @brief Sets whether the timer is running. If the timer is running, it will emit the triggered signal every interval milliseconds,
        /// regardless of the execution time of the slot.
        /// @param x Whether the timer is running.
        void setRunning(bool x) noexcept;

        /// @brief Gets whether the timer is running.
        /// @return Whether the timer is running.
        [[nodiscard]] bool getRunning() const noexcept;

        /// @brief Sets whether the timer should repeat. If the timer is not set to repeat, it will stop after emitting the triggered signal once.
        /// @param x Whether the timer should repeat.
        void setRepeat(bool x) noexcept;

        /// @brief Gets whether the timer should repeat.
        /// @return Whether the timer should repeat.
        [[nodiscard]] bool getRepeat() const noexcept;

    signals:
        /// @brief Emitted when the timer is triggered. The timer will emit this signal every interval milliseconds, regardless of the execution time
        /// of the slot.
        /// @param x The interval in milliseconds.
        void intervalChanged(int x);

        /// @brief Emitted when the running state of the timer changes.
        /// @param x Whether the timer is running.
        void runningChanged(bool x);

        /// @brief Emitted when the repeat state of the timer changes.
        /// @param x Whether the timer should repeat.
        void repeatChanged(bool x);

        /// @brief Emitted when the timer is triggered. The timer will emit this signal every interval milliseconds, regardless of the execution time
        /// of the slot.
        void triggered();

    private:
        QTimer timer_;
        std::chrono::steady_clock::time_point start_{};
        std::chrono::steady_clock::duration accumulate_{};
        std::chrono::milliseconds interval_{};
        bool running_{};
        bool repeat_{};
    };
}