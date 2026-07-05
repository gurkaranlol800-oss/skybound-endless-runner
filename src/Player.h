#pragma once
#include "PhysicsBody.h"
#include "Tilemap.h"

// Player-specific control layered on top of a PhysicsBody: input-driven
// horizontal acceleration/friction, and jump with two small forgiveness
// windows that make platforming feel responsive instead of
// pixel-perfect-timing-dependent:
//  - coyote time: jumping still works for a brief moment after walking
//    off a ledge, as if the player hadn't quite left the ground yet.
//  - jump buffering: pressing jump slightly before landing is remembered
//    and fires the instant the player touches ground, instead of being
//    dropped because the press came a few milliseconds too early.
class Player
{
public:
    void Init(float startX, float startY);

    void Update(float dt, bool moveLeftHeld, bool moveRightHeld, bool jumpPressedThisFrame,
                const Tilemap& tilemap);

    // Visual sprite position - use these (with kWidth/kHeight) only for
    // drawing the character.
    float GetX() const { return m_body.GetX() - kCollisionOffsetX; }
    float GetY() const { return m_body.GetY() - kCollisionOffsetY; }
    static constexpr float kWidth = 64.0f;
    static constexpr float kHeight = 64.0f;

    // The actual gameplay collision box, noticeably narrower than the
    // sprite frame and trimmed at the top - the character artwork has a
    // lot of transparent padding around it, and using the full 64x64
    // frame as the hitbox made ledges and walls feel like they had
    // invisible extensions (you could hang far off an edge, or bump into
    // "nothing" near a wall, well before the frame-sized box actually
    // cleared/reached it). Use these for all gameplay overlap checks
    // (pickups, enemies, hazards).
    float GetCollisionX() const { return m_body.GetX(); }
    float GetCollisionY() const { return m_body.GetY(); }
    static constexpr float kCollisionWidth = 32.0f;
    static constexpr float kCollisionHeight = 56.0f;

    float GetVY() const { return m_body.GetVY(); }
    bool IsGrounded() const { return m_body.IsGrounded(); }
    bool IsFacingLeft() const { return m_facingLeft; }

    // For a stomp bounce (landing on an enemy) or similar external
    // vertical impulses.
    void Bounce(float speed) { m_body.SetVerticalVelocity(-speed); }

private:
    // Centered horizontally; trimmed only from the top vertically so the
    // collision box's bottom edge (the feet) stays flush with the visual
    // sprite's bottom edge.
    static constexpr float kCollisionOffsetX = (kWidth - kCollisionWidth) * 0.5f;
    static constexpr float kCollisionOffsetY = kHeight - kCollisionHeight;

    PhysicsBody m_body;
    bool m_facingLeft = false;

    // Counts down from a fixed value; jumping is allowed whenever this is
    // > 0, whether that's because the player is standing on ground right
    // now or briefly after leaving it (coyote time).
    float m_coyoteTimer = 0.0f;
    // Counts down after a jump press; if it's still > 0 when the player
    // next becomes groundable, the buffered jump fires on landing.
    float m_jumpBufferTimer = 0.0f;
};
