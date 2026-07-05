#pragma once
#include <string>
#include <DirectXMath.h>
#include "Texture.h"
#include "Sprite.h"
#include "Shader.h"

// Draws non-negative integers using a small bitmap font (a horizontal
// strip of glyphs '0'-'9'). The glyphs are plain white so they can be
// recolored per-draw via Sprite's tint parameter - the HUD uses this to
// give coins and distance their own colors from the same texture.
class DigitFont
{
public:
    bool Init(ID3D11Device* device, const char* texturePath, int glyphWidth, int glyphHeight);

    // Draws `value` with its top-left corner at (x, y), scaled by `scale`,
    // returning the total on-screen width drawn (handy for laying out
    // whatever comes after the number).
    float DrawNumber(ID3D11DeviceContext* context, Shader& shader, Sprite& sprite,
                      int value, float x, float y, float scale,
                      int screenWidth, int screenHeight,
                      DirectX::XMFLOAT4 tint);

private:
    Texture m_texture;
    int m_glyphWidth = 0;
    int m_glyphHeight = 0;
};
