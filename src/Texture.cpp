#include "Texture.h"

// STB_IMAGE_IMPLEMENTATION must be defined in exactly one .cpp file - this
// is the one, since Texture is the only place we decode images.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Texture::LoadFromFile(ID3D11Device* device, const char* path)
{
    int channelsInFile = 0;
    // Force 4 channels (RGBA) regardless of the source file's format, so
    // the GPU upload path below can always assume 4 bytes per pixel.
    unsigned char* pixels = stbi_load(path, &m_width, &m_height, &channelsInFile, 4);
    if (!pixels)
        return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE; // uploaded once, never modified after
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels;
    initData.SysMemPitch = m_width * 4; // bytes per row

    HRESULT hr = device->CreateTexture2D(&desc, &initData, &m_texture);
    stbi_image_free(pixels);
    if (FAILED(hr))
        return false;

    hr = device->CreateShaderResourceView(m_texture.Get(), nullptr, &m_srv);
    if (FAILED(hr))
        return false;

    D3D11_SAMPLER_DESC samplerDesc = {};
    // Point filtering keeps pixel art crisp instead of blurring it.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&samplerDesc, &m_sampler);
    return SUCCEEDED(hr);
}

void Texture::Bind(ID3D11DeviceContext* context, UINT slot)
{
    ID3D11ShaderResourceView* srv = m_srv.Get();
    context->PSSetShaderResources(slot, 1, &srv);

    ID3D11SamplerState* sampler = m_sampler.Get();
    context->PSSetSamplers(slot, 1, &sampler);
}
