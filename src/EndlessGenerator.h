#pragma once
#include <vector>
#include <utility>
#include <random>
#include "Tilemap.h"

// Procedurally builds an endless level, chunk by chunk, directly into a
// Tilemap: a ground floor with occasional (always-jumpable) pits, some
// floating platforms, spikes, coins, and enemies. Difficulty - pit
// frequency, spike frequency, enemy frequency - ramps up with distance
// and then plateaus, so the game never becomes literally impossible but
// keeps getting harder for a good while.
class EndlessGenerator
{
public:
    struct ChunkSpawns
    {
        std::vector<std::pair<int, int>> pickups; // tile (col, row)
        std::vector<std::pair<int, int>> enemies; // tile (col, row)
    };

    void Init(int heightTiles, unsigned int seed);

    // Generates whatever additional chunks are needed so the map covers
    // at least targetRightEdgeCol columns, returning the pickups/enemies
    // spawned in the newly generated region only.
    ChunkSpawns GenerateAheadTo(Tilemap& tilemap, int targetRightEdgeCol);

    // How far generation has progressed, in tiles - used both as the
    // "you can't outrun the generator" guarantee and as the difficulty
    // ramp's distance measure.
    int GetGeneratedWidth() const { return m_nextCol; }

private:
    void GenerateOneChunk(Tilemap& tilemap, ChunkSpawns& outSpawns);
    bool Chance(float probability);
    int RandInt(int lo, int hi);
    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    int m_heightTiles = 0;
    int m_nextCol = 0;
    std::mt19937 m_rng;
};
