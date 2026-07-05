#include "AnimationController.h"
#include <utility>

void AnimationController::AddClip(const std::string& name, Texture* texture, Animation animation)
{
    m_clips[name] = Clip{ texture, std::move(animation) };

    // The first clip registered becomes the initial state, so callers
    // don't have to remember to call Play() before the first Update().
    if (m_currentClipName.empty())
        m_currentClipName = name;
}

void AnimationController::Play(const std::string& name)
{
    if (m_currentClipName == name)
        return;

    m_currentClipName = name;
    m_elapsed = 0.0f;
    m_finished = false;
}

void AnimationController::Update(float dt)
{
    if (m_finished)
        return; // non-looping clip already reached its last frame; hold it

    auto it = m_clips.find(m_currentClipName);
    if (it == m_clips.end())
        return;

    const Animation& anim = it->second.animation;
    m_elapsed += dt;

    if (anim.IsLooping())
    {
        float total = anim.GetTotalDuration();
        if (total > 0.0f && m_elapsed >= total)
            m_elapsed = fmodf(m_elapsed, total);
    }
    else if (m_elapsed >= anim.GetTotalDuration())
    {
        m_elapsed = anim.GetTotalDuration();
        m_finished = true;
    }
}

Texture* AnimationController::GetCurrentTexture() const
{
    auto it = m_clips.find(m_currentClipName);
    return it != m_clips.end() ? it->second.texture : nullptr;
}

RECT AnimationController::GetCurrentFrameRect() const
{
    auto it = m_clips.find(m_currentClipName);
    if (it == m_clips.end())
        return RECT{ 0, 0, 0, 0 };

    const Animation& anim = it->second.animation;
    float cumulative = 0.0f;
    int frameCount = anim.GetFrameCount();

    for (int i = 0; i < frameCount; ++i)
    {
        const AnimationFrame& frame = anim.GetFrame(i);
        cumulative += frame.duration;
        // The last-frame fallback guards against float rounding leaving
        // m_elapsed a hair past the final frame's cumulative duration.
        if (m_elapsed < cumulative || i == frameCount - 1)
            return frame.sourceRect;
    }

    return RECT{ 0, 0, 0, 0 };
}
