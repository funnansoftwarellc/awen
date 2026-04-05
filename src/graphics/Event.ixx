module;

#include <cstdint>
#include <variant>

#include <raylib.h>

export module awen.graphics.event;

export namespace awen::graphics
{
    struct EventKeyboard
    {
        enum class Key : std::uint16_t
        {
            null_key = KEY_NULL,
            back = KEY_BACK,
            menu = KEY_MENU,
            volume_up = KEY_VOLUME_UP,
            volume_down = KEY_VOLUME_DOWN,
            space = KEY_SPACE,
            apostrophe = KEY_APOSTROPHE,
            comma = KEY_COMMA,
            minus = KEY_MINUS,
            period = KEY_PERIOD,
            slash = KEY_SLASH,
            zero = KEY_ZERO,
            one = KEY_ONE,
            two = KEY_TWO,
            three = KEY_THREE,
            four = KEY_FOUR,
            five = KEY_FIVE,
            six = KEY_SIX,
            seven = KEY_SEVEN,
            eight = KEY_EIGHT,
            nine = KEY_NINE,
            semicolon = KEY_SEMICOLON,
            equal = KEY_EQUAL,
            a = KEY_A,
            b = KEY_B,
            c = KEY_C,
            d = KEY_D,
            e = KEY_E,
            f = KEY_F,
            g = KEY_G,
            h = KEY_H,
            i = KEY_I,
            j = KEY_J,
            k = KEY_K,
            l = KEY_L,
            m = KEY_M,
            n = KEY_N,
            o = KEY_O,
            p = KEY_P,
            q = KEY_Q,
            r = KEY_R,
            s = KEY_S,
            t = KEY_T,
            u = KEY_U,
            v = KEY_V,
            w = KEY_W,
            x = KEY_X,
            y = KEY_Y,
            z = KEY_Z,
            left_bracket = KEY_LEFT_BRACKET,
            backslash = KEY_BACKSLASH,
            right_bracket = KEY_RIGHT_BRACKET,
            grave = KEY_GRAVE,
            escape = KEY_ESCAPE,
            enter = KEY_ENTER,
            tab = KEY_TAB,
            backspace = KEY_BACKSPACE,
            insert = KEY_INSERT,
            del = KEY_DELETE,
            right = KEY_RIGHT,
            left = KEY_LEFT,
            down = KEY_DOWN,
            up = KEY_UP,
            page_up = KEY_PAGE_UP,
            page_down = KEY_PAGE_DOWN,
            home = KEY_HOME,
            end = KEY_END,
            caps_lock = KEY_CAPS_LOCK,
            scroll_lock = KEY_SCROLL_LOCK,
            num_lock = KEY_NUM_LOCK,
            print_screen = KEY_PRINT_SCREEN,
            pause_key = KEY_PAUSE,
            f1 = KEY_F1,
            f2 = KEY_F2,
            f3 = KEY_F3,
            f4 = KEY_F4,
            f5 = KEY_F5,
            f6 = KEY_F6,
            f7 = KEY_F7,
            f8 = KEY_F8,
            f9 = KEY_F9,
            f10 = KEY_F10,
            f11 = KEY_F11,
            f12 = KEY_F12,
            kp_0 = KEY_KP_0,
            kp_1 = KEY_KP_1,
            kp_2 = KEY_KP_2,
            kp_3 = KEY_KP_3,
            kp_4 = KEY_KP_4,
            kp_5 = KEY_KP_5,
            kp_6 = KEY_KP_6,
            kp_7 = KEY_KP_7,
            kp_8 = KEY_KP_8,
            kp_9 = KEY_KP_9,
            kp_decimal = KEY_KP_DECIMAL,
            kp_divide = KEY_KP_DIVIDE,
            kp_multiply = KEY_KP_MULTIPLY,
            kp_subtract = KEY_KP_SUBTRACT,
            kp_add = KEY_KP_ADD,
            kp_enter = KEY_KP_ENTER,
            kp_equal = KEY_KP_EQUAL,
            left_shift = KEY_LEFT_SHIFT,
            left_control = KEY_LEFT_CONTROL,
            left_alt = KEY_LEFT_ALT,
            left_super = KEY_LEFT_SUPER,
            right_shift = KEY_RIGHT_SHIFT,
            right_control = KEY_RIGHT_CONTROL,
            right_alt = KEY_RIGHT_ALT,
            right_super = KEY_RIGHT_SUPER,
            kb_menu = KEY_KB_MENU,
        };

        enum class Type : std::uint8_t
        {
            pressed,
            released,
        };

        Key key{};
        Type type{};
    };

    struct EventMouse
    {
        enum class MouseButton : std::uint8_t
        {
            left = MOUSE_BUTTON_LEFT,
            right = MOUSE_BUTTON_RIGHT,
            middle = MOUSE_BUTTON_MIDDLE,
            side = MOUSE_BUTTON_SIDE,
            extra = MOUSE_BUTTON_EXTRA,
            forward = MOUSE_BUTTON_FORWARD,
            back = MOUSE_BUTTON_BACK,
        };

        enum class Type : std::uint8_t
        {
            pressed,
            released,
            moved,
            scrolled,
        };

        MouseButton button{};
        Type type{};
        float x{};
        float y{};
        float scroll_x{};
        float scroll_y{};
    };

    struct EventTouch
    {
        enum class Type : std::uint8_t
        {
            pressed,
            released,
            moved,
        };

        int id{};
        Type type{};
        float x{};
        float y{};
    };

    struct EventJoystick
    {
        enum class Button : std::uint8_t
        {
            unknown = GAMEPAD_BUTTON_UNKNOWN,
            left_face_up = GAMEPAD_BUTTON_LEFT_FACE_UP,
            left_face_right = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
            left_face_down = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
            left_face_left = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
            right_face_up = GAMEPAD_BUTTON_RIGHT_FACE_UP,
            right_face_right = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
            right_face_down = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
            right_face_left = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
            left_trigger_1 = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
            left_trigger_2 = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
            right_trigger_1 = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
            right_trigger_2 = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
            middle_left = GAMEPAD_BUTTON_MIDDLE_LEFT,
            middle = GAMEPAD_BUTTON_MIDDLE,
            middle_right = GAMEPAD_BUTTON_MIDDLE_RIGHT,
            left_thumb = GAMEPAD_BUTTON_LEFT_THUMB,
            right_thumb = GAMEPAD_BUTTON_RIGHT_THUMB,
        };

        enum class Type : std::uint8_t
        {
            pressed,
            released,
        };

        int id{};
        Button button{};
        Type type{};
        float axis_x{};
        float axis_y{};
    };

    struct EventWindowResize
    {
        int width{};
        int height{};
    };

    using Event = std::variant<EventKeyboard, EventMouse, EventTouch, EventJoystick, EventWindowResize>;
}
