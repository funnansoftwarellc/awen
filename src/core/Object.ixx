module;

#include <sigslot/signal.hpp>

export module awen.core.object;

export namespace awen::core
{
    class Object
    {
    public:
        virtual ~Object()
        {
            onDestroyed();
        }

        sigslot::signal<> onDestroyed;
    };
}