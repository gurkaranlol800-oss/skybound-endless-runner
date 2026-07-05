#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <utility>
#include <d3d11.h>
#include "Shader.h"
#include "Texture.h"
#include "Sprite.h"

// Tile-space (col, row) spawn points read out of a level file, so
// gameplay code (main.cpp) knows where to place the player and create
// enemies/pickups without needing to scan the tile grid itself.
struct LevelSpawns
{
    int playerCol = 0;
    int playerRow = 0;
    std::vector<std::pair<int, int>> pickups;
    std::vector<std::pair<int, int>> enemies;
};

// A grid of solid/empty/hazard tiles used for both collision queries and
// rendering. Storage is column-major (each column is `height` contiguous
// entries) specifically so the width can grow cheaply by appending whole
// columns - the endless mode extends the map to the right forever as the
// player advances, and column-major storage makes that an O(height)
// append instead of an O(width*height) rebuild.
//
// Two ways to populate one:
//  - LoadFromFile parses a small hand-rolled text format (see below) for
//    a fixed, hand-authored level.
//  - InitEmpty + EnsureWidth + SetSolid/SetHazard let a generator
//    (EndlessGenerator) build the map procedurally, chunk by chunk.
//
// Text format:
//   <width> <height>
//   <height rows of width characters>
//   SPAWN <col> <row>
//
// Row characters: '#' solid ground, '.' empty air, '^' hazard (deadly,
// non-solid), 'o' coin pickup spawn, 'e' enemy spawn (both non-solid;
// recorded into LevelSpawns and treated as empty air for collision).
//
// This is a deliberately simple custom format rather than something like
// Tiled's XML-based .tmx - easy to hand-edit and parse with plain
// std::ifstream, no XML library needed.
class Tilemap
{
public:
    static constexpr int kTileSize = 40;

    bool LoadFromFile(const std::string& path, LevelSpawns& outSpawns);

    // Starts an empty map of the given height and zero width, ready for
    // EnsureWidth/SetSolid/SetHazard to build it up procedurally.
    void InitEmpty(int heightTiles);

    // Grows the map to at least newWidth columns, filling any newly
    // added columns with empty/non-hazard tiles. A no-op if the map is
    // already at least that wide.
    void EnsureWidth(int newWidth);

    void SetSolid(int tileX, int tileY, bool solid);
    void SetHazard(int tileX, int tileY, bool hazard);

    // Out-of-bounds tiles are treated as open air rather than walls, so
    // the player can walk/fall past the edges of the loaded area.
    bool IsSolid(int tileX, int tileY) const;
    bool IsHazard(int tileX, int tileY) const;

    // True if any hazard tile overlaps the given world-space AABB - used
    // for player-vs-spikes checks without duplicating tile-range math at
    // every call site.
    bool OverlapsHazard(float x, float y, float width, float height) const;

    int GetWidthTiles() const { return m_width; }
    int GetHeightTiles() const { return m_height; }

    // Only draws tiles visible within the camera's [cameraX, cameraX +
    // screenWidth) x [cameraY, cameraY + screenHeight) window - levels
    // can be much larger than one screen, so drawing every tile every
    // frame regardless of visibility would be wasteful.
    void Draw(ID3D11DeviceContext* context, Shader& shader, Texture& tileTexture, Texture& hazardTexture,
              Sprite& sprite, float cameraX, float cameraY, int screenWidth, int screenHeight) const;

private:
    size_t Index(int tileX, int tileY) const { return static_cast<size_t>(tileX) * m_height + tileY; }

    int m_width = 0;
    int m_height = 0;
    std::vector<uint8_t> m_tiles;   // column-major; 1 = solid, 0 = empty
    std::vector<uint8_t> m_hazards; // column-major; 1 = hazard, 0 = safe
};
