#include "Pickup.h"
#include <cmath>

namespace
{
    constexpr float kBobAmplitude = 6.0f;  // px
    constexpr float kBobSpeed = 3.0f;      // radians/s
}

void Pickup::Init(float x, float y)
{
    m_x = x;
    m_y = y;
}

void Pickup::Update(float dt)
{
    m_bobTime += dt * kBobSpeed;
    m_bobOffset = std::sin(m_bobTime) * kBobAmplitude;
}
