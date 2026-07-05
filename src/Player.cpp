#include "Player.h"
#include <algorithm>

namespace
{
    constexpr float kMoveSpeed = 300.0f;      // px/s, top horizontal speed
    constexpr float kAcceleration = 2400.0f;  // px/s^2, ramping up to top speed
    constexpr float kFriction = 3000.0f;      // px/s^2, slowing to a stop with no input
    constexpr float kJumpSpeed = 900.0f;      // px/s, initial upward velocity
    constexpr float kCoyoteTime = 0.10f;      // seconds of post-ledge jump grace
    constexpr float kJumpBufferTime = 0.12f;  // seconds a jump press is remembered
}

void Player::Init(float startX, float startY)
{
    m_body.Init(startX + kCollisionOffsetX, startY + kCollisionOffsetY, kCollisionWidth, kCollisionHeight);
}

void Player::Update(float dt, bool moveLeftHeld, bool moveRightHeld, bool jumpPressedThisFrame,
                     const Tilemap& tilemap)
{
    float targetSpeed = 0.0f;
    if (moveLeftHeld)  targetSpeed -= kMoveSpeed;
    if (moveRightHeld) targetSpeed += kMoveSpeed;

    if (moveLeftHeld && !moveRightHeld)
        m_facingLeft = true;
    else if (moveRightHeld && !moveLeftHeld)
        m_facingLeft = false;

    float accel = (targetSpeed != 0.0f) ? kAcceleration : kFriction;

    // Both windows are countdown timers rather than one-frame checks, so
    // a jump succeeds as long as the press and the "allowed" state
    // overlap at all within a short grace period, not only on the exact
    // same frame.
    m_coyoteTimer = m_body.IsGrounded() ? kCoyoteTime : std::max(0.0f, m_coyoteTimer - dt);
    m_jumpBufferTimer = jumpPressedThisFrame ? kJumpBufferTime : std::max(0.0f, m_jumpBufferTimer - dt);

    if (m_jumpBufferTimer > 0.0f && m_coyoteTimer > 0.0f)
    {
        m_body.SetVerticalVelocity(-kJumpSpeed);
        m_jumpBufferTimer = 0.0f;
        m_coyoteTimer = 0.0f;
    }

    m_body.Step(dt, targetSpeed, accel, tilemap);
}
