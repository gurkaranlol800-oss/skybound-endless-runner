#include "PhysicsBody.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kGravity = 2000.0f;      // px/s^2
    constexpr float kMaxFallSpeed = 1200.0f; // px/s, terminal velocity
}

void PhysicsBody::Init(float x, float y, float width, float height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    // Reset motion/contact state too, not just position - Init() is also
    // called to reuse an existing body for a new run (endless mode
    // restart), where stale velocity from the previous run must not
    // carry over.
    m_vx = 0.0f;
    m_vy = 0.0f;
    m_grounded = false;
    m_hitWallLeft = false;
    m_hitWallRight = false;
}

void PhysicsBody::Step(float dt, float targetVx, float accel, const Tilemap& tilemap)
{
    if (accel <= 0.0f)
    {
        m_vx = targetVx;
    }
    else if (m_vx < targetVx)
    {
        m_vx = std::min(m_vx + accel * dt, targetVx);
    }
    else if (m_vx > targetVx)
    {
        m_vx = std::max(m_vx - accel * dt, targetVx);
    }

    m_vy += kGravity * dt;
    if (m_vy > kMaxFallSpeed)
        m_vy = kMaxFallSpeed;

    m_hitWallLeft = false;
    m_hitWallRight = false;

    // Resolve X fully before Y (rather than moving diagonally and testing
    // both at once) - this is what keeps things from clipping into
    // corners when moving into a wall while falling.
    MoveAndCollideX(m_vx * dt, tilemap);
    MoveAndCollideY(m_vy * dt, tilemap);
}

void PhysicsBody::MoveAndCollideX(float dx, const Tilemap& tilemap)
{
    m_x += dx;
    if (dx == 0.0f)
        return;

    int tileSize = Tilemap::kTileSize;
    int top = static_cast<int>(std::floor(m_y / tileSize));
    int bottom = static_cast<int>(std::floor((m_y + m_height - 1.0f) / tileSize));

    if (dx > 0.0f)
    {
        int right = static_cast<int>(std::floor((m_x + m_width) / tileSize));
        for (int ty = top; ty <= bottom; ++ty)
        {
            if (tilemap.IsSolid(right, ty))
            {
                m_x = static_cast<float>(right * tileSize) - m_width;
                m_vx = 0.0f;
                m_hitWallRight = true;
                break;
            }
        }
    }
    else
    {
        int left = static_cast<int>(std::floor(m_x / tileSize));
        for (int ty = top; ty <= bottom; ++ty)
        {
            if (tilemap.IsSolid(left, ty))
            {
                m_x = static_cast<float>((left + 1) * tileSize);
                m_vx = 0.0f;
                m_hitWallLeft = true;
                break;
            }
        }
    }
}

void PhysicsBody::MoveAndCollideY(float dy, const Tilemap& tilemap)
{
    m_y += dy;
    m_grounded = false;

    if (dy == 0.0f)
        return;

    int tileSize = Tilemap::kTileSize;
    int left = static_cast<int>(std::floor(m_x / tileSize));
    int right = static_cast<int>(std::floor((m_x + m_width - 1.0f) / tileSize));

    if (dy > 0.0f)
    {
        int bottom = static_cast<int>(std::floor((m_y + m_height) / tileSize));
        for (int tx = left; tx <= right; ++tx)
        {
            if (tilemap.IsSolid(tx, bottom))
            {
                m_y = static_cast<float>(bottom * tileSize) - m_height;
                m_vy = 0.0f;
                m_grounded = true;
                break;
            }
        }
    }
    else
    {
        int top = static_cast<int>(std::floor(m_y / tileSize));
        for (int tx = left; tx <= right; ++tx)
        {
            if (tilemap.IsSolid(tx, top))
            {
                m_y = static_cast<float>((top + 1) * tileSize);
                m_vy = 0.0f;
                break;
            }
        }
    }
}
