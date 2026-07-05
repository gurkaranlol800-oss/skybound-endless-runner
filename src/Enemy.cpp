#include "Enemy.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr float kPatrolSpeed = 100.0f; // px/s
    constexpr float kChaseSpeed = 180.0f;  // px/s, once the player's spotted
    constexpr float kLedgeProbeDistance = 4.0f; // px past the leading edge to check for ground

    // The player is "spotted" when within this horizontal distance and
    // roughly the same vertical level (close enough to be on the same
    // platform or one just above/below) - a simple, cheap stand-in for
    // proper line-of-sight.
    constexpr float kDetectRangeX = 350.0f;
    constexpr float kDetectRangeY = 90.0f;

    // How long a blocked chase attempt is "forgotten" before the enemy
    // will try chasing again - long enough to stop the frame-by-frame
    // flip-flop that otherwise looks like the enemy vibrating in place
    // at a ledge/wall it can't cross to reach the player.
    constexpr float kChaseCooldownDuration = 1.0f;
}

void Enemy::Init(float startX, float startY)
{
    m_body.Init(startX + kCollisionOffsetX, startY + kCollisionOffsetY, kCollisionWidth, kCollisionHeight);
}

void Enemy::Update(float dt, const Tilemap& tilemap, float playerCenterX, float playerCenterY)
{
    if (!m_alive)
        return;

    m_chaseCooldownTimer = std::max(0.0f, m_chaseCooldownTimer - dt);

    float enemyCenterX = m_body.GetX() + m_body.GetWidth() * 0.5f;
    float enemyCenterY = m_body.GetY() + m_body.GetHeight() * 0.5f;
    float dx = playerCenterX - enemyCenterX;

    bool playerSpotted = m_chaseCooldownTimer <= 0.0f &&
                          std::fabs(dx) < kDetectRangeX &&
                          std::fabs(playerCenterY - enemyCenterY) < kDetectRangeY;

    float desiredDirection = playerSpotted ? (dx > 0.0f ? 1.0f : -1.0f) : m_direction;
    float speed = playerSpotted ? kChaseSpeed : kPatrolSpeed;

    // Look one probe-distance past the leading edge, one tile below the
    // feet, in whichever direction we're about to move: if there's no
    // ground there, don't step off - turn around instead, even if that
    // means giving up a chase.
    float probeX = (desiredDirection > 0.0f) ? (m_body.GetX() + m_body.GetWidth() + kLedgeProbeDistance)
                                              : (m_body.GetX() - kLedgeProbeDistance);
    int tileCol = static_cast<int>(std::floor(probeX / Tilemap::kTileSize));
    int tileRow = static_cast<int>(std::floor((m_body.GetY() + m_body.GetHeight() + 1.0f) / Tilemap::kTileSize));
    bool groundAhead = tilemap.IsSolid(tileCol, tileRow);

    if (m_body.IsGrounded() && !groundAhead)
    {
        desiredDirection = -desiredDirection;
        speed = kPatrolSpeed;
        if (playerSpotted)
            m_chaseCooldownTimer = kChaseCooldownDuration;
    }

    m_direction = desiredDirection;

    // accel <= 0 makes PhysicsBody::Step snap velocity directly instead
    // of ramping up - simple, snappy back-and-forth patrol/chase movement.
    m_body.Step(dt, speed * m_direction, 0.0f, tilemap);

    if (m_body.HitWallLeft() || m_body.HitWallRight())
    {
        m_direction = -m_direction;
        if (playerSpotted)
            m_chaseCooldownTimer = kChaseCooldownDuration;
    }
}
