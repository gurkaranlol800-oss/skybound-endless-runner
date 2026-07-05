#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Compiles an HLSL file containing both a vertex shader (VSMain) and a
// pixel shader (PSMain) entry point, and builds the input layout that
// tells the GPU how to interpret our raw vertex buffer bytes.
class Shader
{
public:
    bool LoadFromFile(ID3D11Device* device, const wchar_t* path);

    void Bind(ID3D11DeviceContext* context);

private:
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
};
