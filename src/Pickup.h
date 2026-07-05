#pragma once

// A static collectible (coin). No physics - it just floats in place with
// a small bobbing offset for visual life, and gets marked collected once
// the player overlaps it.
class Pickup
{
public:
    void Init(float x, float y);
    void Update(float dt);

    float GetX() const { return m_x; }
    // Includes the bobbing offset, so rendering and overlap checks both
    // see the same visual position.
    float GetY() const { return m_y + m_bobOffset; }

    bool IsCollected() const { return m_collected; }
    void Collect() { m_collected = true; }

    static constexpr float kSize = 32.0f;

private:
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_bobOffset = 0.0f;
    float m_bobTime = 0.0f;
    bool m_collected = false;
};
