#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "Shader.h"
#include "Texture.h"

using Microsoft::WRL::ComPtr;

// One vertex of a sprite quad: a position in local space and a UV
// coordinate into the texture. This layout must match Shader.cpp's
// D3D11_INPUT_ELEMENT_DESC array.
struct SpriteVertex
{
    float x, y, z;
    float u, v;
};

// A single 2D textured quad, drawn in screen-pixel space (0,0 = top-left,
// +X = right, +Y = down) so gameplay code can think in ordinary screen
// coordinates instead of normalized device coordinates.
class Sprite
{
public:
    // Creates the (shared-shape) unit quad geometry and constant buffer.
    // Called once; the same quad is reused and repositioned for every draw.
    bool Init(ID3D11Device* device);

    // sourceRectPixels selects which frame of the texture to draw, in pixel
    // coordinates within that texture (e.g. one cell of a sprite sheet).
    // flipHorizontal mirrors the sprite left-to-right in place (same x,
    // width footprint) - used for facing direction. tint is multiplied
    // into the sampled color, e.g. to recolor a white/grayscale texture
    // (like the HUD digit font) per-draw without needing separate
    // texture variants per color.
    void Draw(ID3D11DeviceContext* context, Shader& shader, Texture& texture,
              float x, float y, float width, float height,
              int screenWidth, int screenHeight,
              const RECT& sourceRectPixels, bool flipHorizontal = false,
              DirectX::XMFLOAT4 tint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

private:
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11Buffer> m_constantBuffer;
};
