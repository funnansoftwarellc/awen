#include "../TimerFixed.hpp"

#include <gtest/gtest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>


using awen::core::TimerFixed;

TEST(TimerFixed, DefaultState)
{
    TimerFixed timer;

    EXPECT_EQ(timer.getInterval(), 0);
    EXPECT_FALSE(timer.getRunning());
    EXPECT_FALSE(timer.getRepeat());
}

TEST(TimerFixed, SetIntervalRoundTrip)
{
    TimerFixed timer;

    QSignalSpy spy(&timer, &TimerFixed::intervalChanged);

    timer.setInterval(100);

    EXPECT_EQ(timer.getInterval(), 100);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 100);
}

TEST(TimerFixed, SetIntervalNoSignalWhenUnchanged)
{
    TimerFixed timer;
    timer.setInterval(50);

    QSignalSpy spy(&timer, &TimerFixed::intervalChanged);

    timer.setInterval(50);

    EXPECT_EQ(spy.count(), 0);
}

TEST(TimerFixed, SetRunningRoundTrip)
{
    TimerFixed timer;
    timer.setInterval(1000);

    QSignalSpy spy(&timer, &TimerFixed::runningChanged);

    timer.setRunning(true);

    EXPECT_TRUE(timer.getRunning());
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toBool(), true);

    timer.setRunning(false);

    EXPECT_FALSE(timer.getRunning());
    ASSERT_EQ(spy.count(), 2);
    EXPECT_EQ(spy.at(1).at(0).toBool(), false);
}

TEST(TimerFixed, SetRunningNoSignalWhenUnchanged)
{
    TimerFixed timer;
    timer.setInterval(1000);
    timer.setRunning(true);

    QSignalSpy spy(&timer, &TimerFixed::runningChanged);

    timer.setRunning(true);

    EXPECT_EQ(spy.count(), 0);

    timer.setRunning(false);
}

TEST(TimerFixed, SetRepeatRoundTrip)
{
    TimerFixed timer;

    QSignalSpy spy(&timer, &TimerFixed::repeatChanged);

    timer.setRepeat(true);

    EXPECT_TRUE(timer.getRepeat());
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toBool(), true);
}

TEST(TimerFixed, SetRepeatNoSignalWhenUnchanged)
{
    TimerFixed timer;
    timer.setRepeat(true);

    QSignalSpy spy(&timer, &TimerFixed::repeatChanged);

    timer.setRepeat(true);

    EXPECT_EQ(spy.count(), 0);
}

TEST(TimerFixed, TriggeredFiresAfterInterval)
{
    TimerFixed timer;
    timer.setInterval(50);
    timer.setRepeat(true);

    QSignalSpy triggered(&timer, &TimerFixed::triggered);

    timer.setRunning(true);

    QEventLoop loop;

    // Guard: quit the loop after 500 ms whether or not the signal fires.
    QTimer::singleShot(500, &loop, &QEventLoop::quit);

    // Quit as soon as the first triggered signal arrives.
    QObject::connect(&timer, &TimerFixed::triggered, &loop, &QEventLoop::quit);

    loop.exec();

    timer.setRunning(false);

    EXPECT_GE(triggered.count(), 1);
}

TEST(TimerFixed, NonRepeatStopsAfterOneTrigger)
{
    TimerFixed timer;
    timer.setInterval(50);
    timer.setRepeat(false);

    QSignalSpy triggered(&timer, &TimerFixed::triggered);
    QSignalSpy running(&timer, &TimerFixed::runningChanged);

    timer.setRunning(true);

    QEventLoop loop;

    // Guard: quit after 500 ms.
    QTimer::singleShot(500, &loop, &QEventLoop::quit);

    // Quit once the timer stops itself.
    QObject::connect(&timer, &TimerFixed::runningChanged, &loop,
                     [&loop](bool isRunning)
                     {
                         if (!isRunning)
                         {
                             loop.quit();
                         }
                     });

    loop.exec();

    EXPECT_EQ(triggered.count(), 1);
    EXPECT_FALSE(timer.getRunning());
}
