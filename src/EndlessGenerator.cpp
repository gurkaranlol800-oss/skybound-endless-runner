#include "EndlessGenerator.h"
#include <algorithm>

namespace
{
    constexpr int kChunkWidth = 16;
    // No pits/spikes/enemies before this column, so the player always has
    // a safe run-up right after spawning or restarting.
    constexpr int kSafeStartCols = 14;
    constexpr int kMaxPitWidth = 4; // tiles - always clearable with a single jump
    constexpr int kMinEnemySpacing = 4; // min columns between two enemy spawns
    constexpr float kPlatformEnemyChance = 0.30f; // a platform spawns an enemy instead of a coin

    // Difficulty ramps linearly over this many tiles of distance, then
    // holds steady - the run keeps getting harder for a while but never
    // becomes unfair-by-design no matter how long someone survives.
    constexpr float kDifficultyRampCols = 500.0f;
}

void EndlessGenerator::Init(int heightTiles, unsigned int seed)
{
    m_heightTiles = heightTiles;
    m_nextCol = 0;
    m_rng.seed(seed);
}

bool EndlessGenerator::Chance(float probability)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(m_rng) < probability;
}

int EndlessGenerator::RandInt(int lo, int hi)
{
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(m_rng);
}

EndlessGenerator::ChunkSpawns EndlessGenerator::GenerateAheadTo(Tilemap& tilemap, int targetRightEdgeCol)
{
    ChunkSpawns allSpawns;
    while (m_nextCol < targetRightEdgeCol)
    {
        ChunkSpawns chunkSpawns;
        GenerateOneChunk(tilemap, chunkSpawns);
        allSpawns.pickups.insert(allSpawns.pickups.end(), chunkSpawns.pickups.begin(), chunkSpawns.pickups.end());
        allSpawns.enemies.insert(allSpawns.enemies.end(), chunkSpawns.enemies.begin(), chunkSpawns.enemies.end());
    }
    return allSpawns;
}

void EndlessGenerator::GenerateOneChunk(Tilemap& tilemap, ChunkSpawns& outSpawns)
{
    int startCol = m_nextCol;
    int targetEndCol = startCol + kChunkWidth;
    int groundRow = m_heightTiles - 2;

    float t = std::min(1.0f, static_cast<float>(startCol) / kDifficultyRampCols);
    float pitChance = Lerp(0.04f, 0.28f, t);
    float spikeChance = Lerp(0.02f, 0.16f, t);
    float enemyChance = Lerp(0.12f, 0.38f, t);
    constexpr float kPlatformChance = 0.30f;
    constexpr float kGroundCoinChance = 0.12f;

    bool safeZone = startCol < kSafeStartCols;

    int col = startCol;
    int colsSinceEnemy = kMinEnemySpacing;

    while (col < targetEndCol)
    {
        if (!safeZone && col > startCol && Chance(pitChance))
        {
            int pitWidth = RandInt(2, kMaxPitWidth);
            for (int i = 0; i < pitWidth; ++i)
            {
                tilemap.EnsureWidth(col + 1); // leaves the new column empty - the pit itself
                ++col;
            }
            ++colsSinceEnemy;
            continue;
        }

        tilemap.EnsureWidth(col + 1);
        tilemap.SetSolid(col, groundRow, true);
        tilemap.SetSolid(col, groundRow + 1, true);

        if (!safeZone && Chance(spikeChance))
        {
            tilemap.SetHazard(col, groundRow - 1, true);
        }
        else if (!safeZone && colsSinceEnemy >= kMinEnemySpacing && Chance(enemyChance))
        {
            // Spawned a few tiles above the ground and left to fall - the
            // same PhysicsBody gravity/collision used everywhere else
            // settles it onto the surface without needing to compute the
            // exact resting row here.
            outSpawns.enemies.emplace_back(col, groundRow - 3);
            colsSinceEnemy = 0;
        }
        else if (Chance(kPlatformChance) && col + 5 < targetEndCol)
        {
            // Capped well under the player's max jump height (~5 tiles,
            // from kJumpSpeed/kGravity in PhysicsBody) - platforms placed
            // higher than that were unreachable, silently wasting the
            // coin spawned on top of them.
            int platformRow = groundRow - RandInt(2, 4);
            int platformWidth = RandInt(3, 5);
            for (int i = 0; i < platformWidth; ++i)
            {
                tilemap.EnsureWidth(col + i + 1);
                tilemap.SetSolid(col + i, platformRow, true);
            }

            if (!safeZone && colsSinceEnemy >= kMinEnemySpacing && Chance(kPlatformEnemyChance))
            {
                // A few tiles above the platform, same "let gravity settle
                // it" approach used for ground enemies.
                outSpawns.enemies.emplace_back(col + platformWidth / 2, platformRow - 3);
                colsSinceEnemy = 0;
            }
            else
            {
                outSpawns.pickups.emplace_back(col + platformWidth / 2, platformRow - 1);
            }

            col += platformWidth;
            ++colsSinceEnemy;
            continue;
        }
        else if (Chance(kGroundCoinChance))
        {
            outSpawns.pickups.emplace_back(col, groundRow - 1);
        }

        ++col;
        ++colsSinceEnemy;
    }

    m_nextCol = col;
}
