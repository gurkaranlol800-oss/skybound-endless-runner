#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr - auto-releasing COM pointer

using Microsoft::WRL::ComPtr;

// Owns the Direct3D 11 device, swap chain, and per-frame render target.
// This is the "GPU glue" layer: everything later (shaders, sprites) draws
// through the device/context this class creates.
class Renderer
{
public:
    bool Init(HWND hwnd, int width, int height);

    void BeginFrame(float r, float g, float b, float a);
    void EndFrame(); // presents the back buffer to the window

    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11RasterizerState> m_rasterizerState;
    ComPtr<ID3D11BlendState> m_blendState;

    int m_width = 0;
    int m_height = 0;
};
