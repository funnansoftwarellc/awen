module;

#include <cstdint>
#include <variant>

#include <SDL3/SDL_keycode.h>

export module awen.sdl.event;

export namespace awen::sdl
{
    struct EventKeyboard
    {
        enum class Key : std::uint32_t
        {
            unknown = SDLK_UNKNOWN,
            space = SDLK_SPACE,
            apostrophe = SDLK_APOSTROPHE,
            comma = SDLK_COMMA,
            minus = SDLK_MINUS,
            period = SDLK_PERIOD,
            slash = SDLK_SLASH,
            zero = SDLK_0,
            one = SDLK_1,
            two = SDLK_2,
            three = SDLK_3,
            four = SDLK_4,
            five = SDLK_5,
            six = SDLK_6,
            seven = SDLK_7,
            eight = SDLK_8,
            nine = SDLK_9,
            semicolon = SDLK_SEMICOLON,
            equal = SDLK_EQUALS,
            a = SDLK_A,
            b = SDLK_B,
            c = SDLK_C,
            d = SDLK_D,
            e = SDLK_E,
            f = SDLK_F,
            g = SDLK_G,
            h = SDLK_H,
            i = SDLK_I,
            j = SDLK_J,
            k = SDLK_K,
            l = SDLK_L,
            m = SDLK_M,
            n = SDLK_N,
            o = SDLK_O,
            p = SDLK_P,
            q = SDLK_Q,
            r = SDLK_R,
            s = SDLK_S,
            t = SDLK_T,
            u = SDLK_U,
            v = SDLK_V,
            w = SDLK_W,
            x = SDLK_X,
            y = SDLK_Y,
            z = SDLK_Z,
            left_bracket = SDLK_LEFTBRACKET,
            backslash = SDLK_BACKSLASH,
            right_bracket = SDLK_RIGHTBRACKET,
            grave = SDLK_GRAVE,
            escape = SDLK_ESCAPE,
            enter = SDLK_RETURN,
            tab = SDLK_TAB,
            backspace = SDLK_BACKSPACE,
            insert = SDLK_INSERT,
            del = SDLK_DELETE,
            right = SDLK_RIGHT,
            left = SDLK_LEFT,
            down = SDLK_DOWN,
            up = SDLK_UP,
            page_up = SDLK_PAGEUP,
            page_down = SDLK_PAGEDOWN,
            home = SDLK_HOME,
            end = SDLK_END,
            caps_lock = SDLK_CAPSLOCK,
            scroll_lock = SDLK_SCROLLLOCK,
            num_lock = SDLK_NUMLOCKCLEAR,
            print_screen = SDLK_PRINTSCREEN,
            pause_key = SDLK_PAUSE,
            f1 = SDLK_F1,
            f2 = SDLK_F2,
            f3 = SDLK_F3,
            f4 = SDLK_F4,
            f5 = SDLK_F5,
            f6 = SDLK_F6,
            f7 = SDLK_F7,
            f8 = SDLK_F8,
            f9 = SDLK_F9,
            f10 = SDLK_F10,
            f11 = SDLK_F11,
            f12 = SDLK_F12,
            kp_0 = SDLK_KP_0,
            kp_1 = SDLK_KP_1,
            kp_2 = SDLK_KP_2,
            kp_3 = SDLK_KP_3,
            kp_4 = SDLK_KP_4,
            kp_5 = SDLK_KP_5,
            kp_6 = SDLK_KP_6,
            kp_7 = SDLK_KP_7,
            kp_8 = SDLK_KP_8,
            kp_9 = SDLK_KP_9,
            kp_decimal = SDLK_KP_PERIOD,
            kp_divide = SDLK_KP_DIVIDE,
            kp_multiply = SDLK_KP_MULTIPLY,
            kp_subtract = SDLK_KP_MINUS,
            kp_add = SDLK_KP_PLUS,
            kp_enter = SDLK_KP_ENTER,
            kp_equal = SDLK_KP_EQUALS,
            left_shift = SDLK_LSHIFT,
            left_control = SDLK_LCTRL,
            left_alt = SDLK_LALT,
            left_super = SDLK_LGUI,
            right_shift = SDLK_RSHIFT,
            right_control = SDLK_RCTRL,
            right_alt = SDLK_RALT,
            right_super = SDLK_RGUI,
            menu = SDLK_MENU,
        };

        enum class Type : std::uint8_t
        {
            pressed,
            released,
        };

        Key key{};
        Type type{};
    };

    struct EventWindowResize
    {
        int width{};
        int height{};
    };

    using Event = std::variant<EventKeyboard, EventWindowResize>;
}