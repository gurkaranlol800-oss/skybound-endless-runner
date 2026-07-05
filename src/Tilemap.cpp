#include "Tilemap.h"
#include <fstream>
#include <algorithm>
#include <cmath>

bool Tilemap::LoadFromFile(const std::string& path, LevelSpawns& outSpawns)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    int width = 0, height = 0;
    file >> width >> height;
    file.ignore(); // skip the rest of the header line's newline

    InitEmpty(height);
    EnsureWidth(width);
    outSpawns = LevelSpawns{};

    for (int row = 0; row < height; ++row)
    {
        std::string line;
        if (!std::getline(file, line))
            return false;

        for (int col = 0; col < width && col < static_cast<int>(line.size()); ++col)
        {
            switch (line[col])
            {
            case '#':
                SetSolid(col, row, true);
                break;
            case '^':
                SetHazard(col, row, true);
                break;
            case 'o':
                outSpawns.pickups.emplace_back(col, row);
                break;
            case 'e':
                outSpawns.enemies.emplace_back(col, row);
                break;
            default:
                break; // '.' and anything else is just empty air
            }
        }
    }

    // Optional trailing "SPAWN <col> <row>" line; defaults to the origin
    // if the file doesn't have one.
    std::string tag;
    if (file >> tag && tag == "SPAWN")
        file >> outSpawns.playerCol >> outSpawns.playerRow;

    return true;
}

void Tilemap::InitEmpty(int heightTiles)
{
    m_width = 0;
    m_height = heightTiles;
    m_tiles.clear();
    m_hazards.clear();
}

void Tilemap::EnsureWidth(int newWidth)
{
    if (newWidth <= m_width)
        return;

    m_tiles.resize(static_cast<size_t>(newWidth) * m_height, 0);
    m_hazards.resize(static_cast<size_t>(newWidth) * m_height, 0);
    m_width = newWidth;
}

void Tilemap::SetSolid(int tileX, int tileY, bool solid)
{
    if (tileX < 0 || tileX >= m_width || tileY < 0 || tileY >= m_height)
        return;
    m_tiles[Index(tileX, tileY)] = solid ? 1 : 0;
}

void Tilemap::SetHazard(int tileX, int tileY, bool hazard)
{
    if (tileX < 0 || tileX >= m_width || tileY < 0 || tileY >= m_height)
        return;
    m_hazards[Index(tileX, tileY)] = hazard ? 1 : 0;
}

bool Tilemap::IsSolid(int tileX, int tileY) const
{
    if (tileX < 0 || tileX >= m_width || tileY < 0 || tileY >= m_height)
        return false;

    return m_tiles[Index(tileX, tileY)] != 0;
}

bool Tilemap::IsHazard(int tileX, int tileY) const
{
    if (tileX < 0 || tileX >= m_width || tileY < 0 || tileY >= m_height)
        return false;

    return m_hazards[Index(tileX, tileY)] != 0;
}

bool Tilemap::OverlapsHazard(float x, float y, float width, float height) const
{
    int left = static_cast<int>(std::floor(x / kTileSize));
    int right = static_cast<int>(std::floor((x + width - 1.0f) / kTileSize));
    int top = static_cast<int>(std::floor(y / kTileSize));
    int bottom = static_cast<int>(std::floor((y + height - 1.0f) / kTileSize));

    for (int ty = top; ty <= bottom; ++ty)
        for (int tx = left; tx <= right; ++tx)
            if (IsHazard(tx, ty))
                return true;

    return false;
}

void Tilemap::Draw(ID3D11DeviceContext* context, Shader& shader, Texture& tileTexture, Texture& hazardTexture,
                    Sprite& sprite, float cameraX, float cameraY, int screenWidth, int screenHeight) const
{
    RECT fullTileTexture = { 0, 0, tileTexture.GetWidth(), tileTexture.GetHeight() };
    RECT fullHazardTexture = { 0, 0, hazardTexture.GetWidth(), hazardTexture.GetHeight() };

    int startCol = std::max(0, static_cast<int>(cameraX) / kTileSize);
    int endCol = std::min(m_width - 1, static_cast<int>(cameraX + screenWidth) / kTileSize);
    int startRow = std::max(0, static_cast<int>(cameraY) / kTileSize);
    int endRow = std::min(m_height - 1, static_cast<int>(cameraY + screenHeight) / kTileSize);

    for (int y = startRow; y <= endRow; ++y)
    {
        for (int x = startCol; x <= endCol; ++x)
        {
            bool solid = IsSolid(x, y);
            bool hazard = IsHazard(x, y);
            if (!solid && !hazard)
                continue;

            sprite.Draw(context, shader, solid ? tileTexture : hazardTexture,
                        x * kTileSize - cameraX, y * kTileSize - cameraY,
                        static_cast<float>(kTileSize), static_cast<float>(kTileSize),
                        screenWidth, screenHeight, solid ? fullTileTexture : fullHazardTexture);
        }
    }
}
