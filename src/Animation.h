#pragma once
#include <Windows.h>
#include <vector>

// A single frame's source rectangle (in pixel coordinates within a sprite
// sheet texture) and how long it stays on screen.
struct AnimationFrame
{
    RECT sourceRect;
    float duration; // seconds
};

// Pure timing/frame data for one animation clip (e.g. "run"). Holds no
// playback state (no current time) so the same Animation can be shared and
// played independently by multiple AnimationControllers if needed later.
class Animation
{
public:
    // Builds a clip from a horizontal strip of equally-sized frames laid
    // out left-to-right in a sprite sheet, each shown for frameDuration
    // seconds - the common case for simple sprite-sheet animation.
    static Animation FromHorizontalStrip(int frameWidth, int frameHeight, int frameCount,
                                          float frameDuration, bool looping);

    int GetFrameCount() const { return static_cast<int>(m_frames.size()); }
    const AnimationFrame& GetFrame(int index) const { return m_frames[index]; }
    float GetTotalDuration() const { return m_totalDuration; }
    bool IsLooping() const { return m_looping; }

private:
    std::vector<AnimationFrame> m_frames;
    float m_totalDuration = 0.0f;
    bool m_looping = true;
};
