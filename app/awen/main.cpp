#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <cstdlib>

auto main(int argc, char* argv[]) -> int
try
{
    auto app = QGuiApplication{argc, argv};
    auto engine = QQmlApplicationEngine{};

    engine.loadFromModule("AweApp", "Main");

    if (engine.rootObjects().isEmpty())
    {
        return EXIT_FAILURE;
    }

    return QGuiApplication::exec();
}
catch (...)
{
    return EXIT_FAILURE;
}