#include "Shader.h"
#include <d3dcompiler.h>
#include <cstdio>

#pragma comment(lib, "d3dcompiler.lib")

static ComPtr<ID3DBlob> CompileShader(const wchar_t* path, const char* entryPoint, const char* target)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        path,
        nullptr, nullptr, // no macros, no #include handler
        entryPoint,
        target,
        flags, 0,
        &shaderBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob)
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        return nullptr;
    }

    return shaderBlob;
}

bool Shader::LoadFromFile(ID3D11Device* device, const wchar_t* path)
{
    ComPtr<ID3DBlob> vsBlob = CompileShader(path, "VSMain", "vs_5_0");
    if (!vsBlob)
        return false;

    ComPtr<ID3DBlob> psBlob = CompileShader(path, "PSMain", "ps_5_0");
    if (!psBlob)
        return false;

    HRESULT hr = device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(hr))
        return false;

    hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    if (FAILED(hr))
        return false;

    // This layout describes SpriteVertex (see Sprite.h): a 3-float position
    // followed by a 2-float UV. It must be validated against the compiled
    // vertex shader's input signature (vsBlob), which is why this happens
    // after compiling the vertex shader rather than being fully independent.
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(
        layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &m_inputLayout
    );

    return SUCCEEDED(hr);
}

void Shader::Bind(ID3D11DeviceContext* context)
{
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}
