#include <gtest/gtest.h>
#include <QCoreApplication>


auto main(int argc, char* argv[]) -> int
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
