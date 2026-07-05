#pragma once

// Simple axis-aligned bounding box overlap test, shared by all the
// entity-vs-entity checks (player-vs-pickup, player-vs-enemy) - tilemap
// collision has its own tile-grid-specific logic in PhysicsBody instead.
inline bool AABBOverlap(float ax, float ay, float aw, float ah,
                         float bx, float by, float bw, float bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
