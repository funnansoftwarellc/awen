#include "TimerFixed.hpp"

using awen::core::TimerFixed;

TimerFixed::TimerFixed(QObject* parent) : QObject(parent)
{
    timer_.setInterval(0);
    timer_.setTimerType(Qt::TimerType::PreciseTimer);

    QObject::connect(&timer_, &QTimer::timeout, this,
                     [this]()
                     {
                         const auto now = std::chrono::steady_clock::now();
                         accumulate_ += (now - start_);
                         start_ = now;

                         while (accumulate_ >= interval_)
                         {
                             accumulate_ -= interval_;
                             emit triggered();

                             if (!repeat_)
                             {
                                 setRunning(false);
                                 break;
                             }
                         }
                     });
}

void TimerFixed::setInterval(int x) noexcept
{
    if (x == interval_.count())
    {
        return;
    }

    interval_ = std::chrono::milliseconds(x);
    emit intervalChanged(interval_.count());
}

int TimerFixed::getInterval() const noexcept
{
    return interval_.count();
}

void TimerFixed::setRunning(bool x) noexcept
{
    if (x == running_)
    {
        return;
    }

    running_ = x;

    if (running_)
    {
        start_ = std::chrono::steady_clock::now();
        timer_.start();
    }
    else
    {
        timer_.stop();
    }

    emit runningChanged(running_);
}

bool TimerFixed::getRunning() const noexcept
{
    return running_;
}

void TimerFixed::setRepeat(bool x) noexcept
{
    if (x == repeat_)
    {
        return;
    }

    repeat_ = x;
    emit repeatChanged(repeat_);
}

bool TimerFixed::getRepeat() const noexcept
{
    return repeat_;
}