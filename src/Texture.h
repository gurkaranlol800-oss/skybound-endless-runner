#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Loads a PNG/JPG/etc. from disk (via stb_image) into a GPU texture the
// pixel shader can sample from.
class Texture
{
public:
    bool LoadFromFile(ID3D11Device* device, const char* path);

    void Bind(ID3D11DeviceContext* context, UINT slot = 0);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_srv;
    ComPtr<ID3D11SamplerState> m_sampler;

    int m_width = 0;
    int m_height = 0;
};
