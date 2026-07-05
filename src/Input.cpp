#include "Input.h"

bool Input::s_currentKeys[256] = {};
bool Input::s_pressLatch[256] = {};

void Input::OnKeyDown(WPARAM key)
{
    if (key < 256)
    {
        s_currentKeys[key] = true;
        s_pressLatch[key] = true;
    }
}

void Input::OnKeyUp(WPARAM key)
{
    if (key < 256)
        s_currentKeys[key] = false;
}

bool Input::IsKeyDown(int virtualKeyCode)
{
    return s_currentKeys[virtualKeyCode];
}

bool Input::WasKeyPressed(int virtualKeyCode)
{
    if (!s_pressLatch[virtualKeyCode])
        return false;

    s_pressLatch[virtualKeyCode] = false;
    return true;
}
