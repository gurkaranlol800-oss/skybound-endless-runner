#include "Renderer.h"

bool Renderer::Init(HWND hwnd, int width, int height)
{
    m_width = width;
    m_height = height;

    // Describes the swap chain: the pair (or more) of back-buffer images
    // the GPU renders into while the previous one is displayed, avoiding
    // tearing/flicker.
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 8 bits per channel RGBA
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1; // no multisampling for now
    scd.Windowed = TRUE;

    UINT deviceFlags = 0;
#if defined(_DEBUG)
    // Enables the D3D debug layer, which reports API misuse in the output
    // window - invaluable while learning D3D11.
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                   // use the default adapter (primary GPU)
        D3D_DRIVER_TYPE_HARDWARE,  // use the real GPU, not a software rasterizer
        nullptr,
        deviceFlags,
        &featureLevel, 1,
        D3D11_SDK_VERSION,
        &scd,
        &m_swapChain,
        &m_device,
        nullptr,
        &m_context
    );

    if (FAILED(hr))
        return false;

    // The render target view is what lets the pipeline draw into the swap
    // chain's back buffer texture.
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr))
        return false;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr))
        return false;

    // The viewport maps normalized device coordinates to actual pixels on
    // the back buffer; without this nothing would be rasterized on screen.
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);

    // 2D sprites are flat quads that always face the camera, and some are
    // drawn mirrored (facing-direction flips), which reverses their
    // triangle winding order. Back-face culling - enabled in D3D11's
    // default rasterizer state - would silently discard those mirrored
    // sprites, so it's turned off entirely rather than relied upon.
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthClipEnable = TRUE;
    hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
    if (FAILED(hr))
        return false;
    m_context->RSSetState(m_rasterizerState.Get());

    // Standard alpha blending. Without this, D3D11's default blend state
    // ignores alpha entirely and writes each pixel's RGB straight over
    // whatever was already there - so a texture's transparent regions
    // (alpha 0) would still paint over the background solidly instead of
    // letting it show through, regardless of how correct the PNG's alpha
    // channel is.
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_device->CreateBlendState(&blendDesc, &m_blendState);
    if (FAILED(hr))
        return false;
    m_context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);

    return true;
}

void Renderer::BeginFrame(float r, float g, float b, float a)
{
    ID3D11RenderTargetView* rtv = m_renderTargetView.Get();
    m_context->OMSetRenderTargets(1, &rtv, nullptr);

    const float clearColor[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}

void Renderer::EndFrame()
{
    // Present with vsync (SyncInterval = 1) to avoid screen tearing and
    // cap the frame rate to the monitor's refresh rate.
    m_swapChain->Present(1, 0);
}
