#pragma once
#include <Windows.h>

// Thin wrapper around the raw Win32 window creation + message loop. This is
// the "OS glue" every Windows program needs before any game code can run:
// register a window class, create the window, and pump its message queue.
class Window
{
public:
    bool Create(HINSTANCE hInstance, int width, int height, const wchar_t* title);

    // Pumps all pending Win32 messages for this window without blocking.
    // Returns false once the window has been closed (WM_QUIT received).
    bool ProcessMessages();

    HWND GetHandle() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_shouldClose = false;
};
