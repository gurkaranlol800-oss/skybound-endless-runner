#pragma once
#include "Tilemap.h"

// Shared physics for anything that stands on the tilemap and falls under
// gravity - both the player and enemies use this, which is the
// "component" half of this engine's simple GameObject/Component split:
// gameplay-specific classes (Player, Enemy) own one of these and layer
// their own input/AI on top of it rather than reimplementing gravity
// and collision each time.
class PhysicsBody
{
public:
    void Init(float x, float y, float width, float height);

    // Accelerates horizontal velocity toward targetVx at the given rate
    // (or snaps instantly if accel <= 0), applies gravity, then resolves
    // movement against the tilemap one axis at a time (X fully before Y)
    // to avoid tunneling into corners.
    void Step(float dt, float targetVx, float accel, const Tilemap& tilemap);

    // Sets vertical velocity directly - used for jumps (player) or a
    // bounce (e.g. stomping an enemy).
    void SetVerticalVelocity(float vy) { m_vy = vy; }

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    float GetVX() const { return m_vx; }
    float GetVY() const { return m_vy; }
    bool IsGrounded() const { return m_grounded; }
    bool HitWallLeft() const { return m_hitWallLeft; }
    bool HitWallRight() const { return m_hitWallRight; }

private:
    void MoveAndCollideX(float dx, const Tilemap& tilemap);
    void MoveAndCollideY(float dy, const Tilemap& tilemap);

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    float m_vx = 0.0f;
    float m_vy = 0.0f;
    bool m_grounded = false;
    bool m_hitWallLeft = false;
    bool m_hitWallRight = false;
};
