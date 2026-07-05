#include "Window.h"
#include "Input.h"

// Win32 identifies window classes by name; this just needs to be unique
// within our process.
static const wchar_t* kWindowClassName = L"GameEngineWindowClass";

bool Window::Create(HINSTANCE hInstance, int width, int height, const wchar_t* title)
{
    m_width = width;
    m_height = height;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW; // redraw on resize; we don't resize yet but it's harmless
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // D3D will own painting the client area, not GDI
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassExW(&wc))
        return false;

    // We ask Windows for a window whose CLIENT area (the drawable part,
    // excluding title bar/borders) is exactly width x height, so our D3D
    // swap chain matches the size we expect.
    RECT rect = { 0, 0, width, height };
    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX; // fixed-size window for now
    AdjustWindowRect(&rect, style, FALSE);

    // A fixed launch position (rather than CW_USEDEFAULT) means the
    // window shows up in the same place run after run, which is a
    // reasonable, predictable default.
    m_hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        title,
        style,
        50, 50,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr,
        hInstance,
        this // passed through to WM_NCCREATE so WndProc can recover `this`
    );

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, SW_SHOW);

    // Explicitly claim keyboard focus. Becoming the foreground window
    // (via user click, Alt-Tab, etc.) doesn't always imply this window
    // also becomes the per-thread focus target that WM_KEYDOWN/WM_KEYUP
    // get routed to - without this, keyboard input can silently fail to
    // reach the window even though it looks active.
    SetFocus(m_hwnd);

    return true;
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            m_shouldClose = true;

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return !m_shouldClose;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Recover the Window instance associated with this HWND. On
    // WM_NCCREATE (the very first message) we stash it in the window's
    // user-data slot; every later message reads it back out.
    Window* self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }

    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        Input::OnKeyDown(wParam);
        return 0;

    case WM_KEYUP:
        Input::OnKeyUp(wParam);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
