#pragma once
#include "PhysicsBody.h"
#include "Tilemap.h"

// A patrolling enemy that turns around at walls/ledges (so it never
// walks off a platform), and breaks from its patrol to chase the player
// when they're within detection range - falling back to patrolling
// again once the player leaves range or a chase would walk it off a
// ledge.
class Enemy
{
public:
    void Init(float startX, float startY);
    void Update(float dt, const Tilemap& tilemap, float playerCenterX, float playerCenterY);

    // Visual sprite position - use these (with kWidth/kHeight) only for
    // drawing.
    float GetX() const { return m_body.GetX() - kCollisionOffsetX; }
    float GetY() const { return m_body.GetY() - kCollisionOffsetY; }
    static constexpr float kWidth = 64.0f;
    static constexpr float kHeight = 64.0f;

    // The actual gameplay collision box - see Player's kCollisionWidth
    // for why this is deliberately smaller than the sprite frame. Use
    // these for all gameplay overlap checks (stomp/hit detection).
    float GetCollisionX() const { return m_body.GetX(); }
    float GetCollisionY() const { return m_body.GetY(); }
    static constexpr float kCollisionWidth = 32.0f;
    static constexpr float kCollisionHeight = 56.0f;

    bool IsFacingLeft() const { return m_direction < 0.0f; }
    bool IsAlive() const { return m_alive; }
    void Kill() { m_alive = false; }

private:
    static constexpr float kCollisionOffsetX = (kWidth - kCollisionWidth) * 0.5f;
    static constexpr float kCollisionOffsetY = kHeight - kCollisionHeight;

    PhysicsBody m_body;
    float m_direction = -1.0f; // -1 = walking left, +1 = walking right
    bool m_alive = true;

    // While > 0, chasing is suppressed (patrol only) - set whenever a
    // chase attempt gets blocked by a ledge or wall, so the enemy
    // commits to backing off instead of re-targeting the player and
    // immediately getting blocked again every single frame (which
    // otherwise looks like rapid back-and-forth vibration in place).
    float m_chaseCooldownTimer = 0.0f;
};
