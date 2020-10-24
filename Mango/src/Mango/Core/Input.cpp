#include "mgpch.h"
#include "Input.h"

namespace Mango {

    bool Input::IsKeyDown(KeyCode keycode)
    {
        return GetKeyState((uint16_t)keycode) & 0x8000;
    }

    bool Input::IsMouseButtonDown(MouseCode mousebutton)
    {
        return GetKeyState((uint16_t)mousebutton) & 0x8000;
        return false;
    }

}