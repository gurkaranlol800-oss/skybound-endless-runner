#pragma once
#include <Windows.h>

// Tracks keyboard state so gameplay code can ask "is this key held right
// now" or "was this key pressed since I last checked" without caring
// about the raw Win32 message stream. The Window's WndProc feeds key
// events in here.
class Input
{
public:
    static void OnKeyDown(WPARAM key);
    static void OnKeyUp(WPARAM key);

    static bool IsKeyDown(int virtualKeyCode);

    // Returns true at most once per key press, regardless of how many
    // render frames occur before gameplay code asks: the first call after
    // a press consumes it (clearing the latch), and later calls return
    // false until the next press. This matters because rendering runs
    // every frame while gameplay logic runs on a fixed timestep - the two
    // are not on the same cadence, so a plain "did the key state change
    // since last frame" comparison can miss a press that happened between
    // gameplay steps. Latching the event at the moment of the Win32
    // message and consuming it whenever gameplay actually checks avoids
    // that race entirely.
    static bool WasKeyPressed(int virtualKeyCode);

private:
    // 256 possible virtual-key codes (VK_*), see Win32 Virtual-Key Codes.
    static bool s_currentKeys[256];
    static bool s_pressLatch[256];
};
