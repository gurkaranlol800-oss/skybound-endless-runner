#include "DigitFont.h"

bool DigitFont::Init(ID3D11Device* device, const char* texturePath, int glyphWidth, int glyphHeight)
{
    m_glyphWidth = glyphWidth;
    m_glyphHeight = glyphHeight;
    return m_texture.LoadFromFile(device, texturePath);
}

float DigitFont::DrawNumber(ID3D11DeviceContext* context, Shader& shader, Sprite& sprite,
                             int value, float x, float y, float scale,
                             int screenWidth, int screenHeight,
                             DirectX::XMFLOAT4 tint)
{
    std::string digits = std::to_string(value < 0 ? 0 : value);

    float drawnWidth = m_glyphWidth * scale;
    float drawnHeight = m_glyphHeight * scale;
    float cursorX = x;

    for (char c : digits)
    {
        int glyphIndex = c - '0';
        RECT sourceRect = {
            glyphIndex * m_glyphWidth, 0,
            (glyphIndex + 1) * m_glyphWidth, m_glyphHeight
        };

        sprite.Draw(context, shader, m_texture,
                    cursorX, y, drawnWidth, drawnHeight,
                    screenWidth, screenHeight, sourceRect, false, tint);

        cursorX += drawnWidth;
    }

    return cursorX - x;
}
