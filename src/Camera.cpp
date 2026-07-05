#include "Camera.h"
#include <algorithm>
#include <cmath>

void Camera::Init(int levelWidthPixels, int levelHeightPixels, int screenWidth, int screenHeight)
{
    m_levelWidthPixels = levelWidthPixels;
    m_levelHeightPixels = levelHeightPixels;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Starts at the level's top-left; Follow()'s smoothing will ease it
    // toward the player's spawn position over the first few frames.
    m_x = 0.0f;
    m_y = 0.0f;
}

void Camera::Follow(float targetCenterX, float targetCenterY, float dt)
{
    float desiredX = targetCenterX - m_screenWidth * 0.5f;
    float desiredY = targetCenterY - m_screenHeight * 0.5f;

    // Clamp so the camera never shows past the level's edges. If the
    // level is smaller than the screen in a dimension, this collapses to
    // [0, 0], pinning that axis in place.
    float maxX = static_cast<float>(std::max(0, m_levelWidthPixels - m_screenWidth));
    float maxY = static_cast<float>(std::max(0, m_levelHeightPixels - m_screenHeight));
    desiredX = std::clamp(desiredX, 0.0f, maxX);
    desiredY = std::clamp(desiredY, 0.0f, maxY);

    // Exponential smoothing toward the desired position instead of
    // snapping instantly - a small, cheap touch that keeps the camera
    // from feeling jittery as the player changes direction.
    constexpr float kSmoothingSpeed = 10.0f;
    float t = 1.0f - std::exp(-kSmoothingSpeed * dt);
    m_x += (desiredX - m_x) * t;
    m_y += (desiredY - m_y) * t;
}
