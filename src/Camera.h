#pragma once

// Tracks a target position (the player) and computes a top-left scroll
// offset so the world can be drawn shifted by (-x, -y), keeping the
// target centered on screen without ever scrolling past the edges of
// the level.
class Camera
{
public:
    void Init(int levelWidthPixels, int levelHeightPixels, int screenWidth, int screenHeight);

    // Updates the right-edge clamp as the level grows (e.g. an endless
    // generator extending the map ahead of the player) without resetting
    // the camera's current position.
    void SetLevelWidth(int levelWidthPixels) { m_levelWidthPixels = levelWidthPixels; }

    // targetCenterX/Y is the world-space point the camera tries to
    // center on - the player's center point, not its top-left corner.
    void Follow(float targetCenterX, float targetCenterY, float dt);

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }

private:
    float m_x = 0.0f;
    float m_y = 0.0f;
    int m_levelWidthPixels = 0;
    int m_levelHeightPixels = 0;
    int m_screenWidth = 0;
    int m_screenHeight = 0;
};
