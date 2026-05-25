#include <awen/test/Test.hpp>

import awen.raylib.events;

UNIT_TEST(Events, MouseButtonUnknownDoesNotAliasLeft)
{
    EXPECT_NE(awen::raylib::EventMouse::Button::Unknown, awen::raylib::EventMouse::Button::Left);
}