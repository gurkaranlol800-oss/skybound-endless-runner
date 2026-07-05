#include "Animation.h"

Animation Animation::FromHorizontalStrip(int frameWidth, int frameHeight, int frameCount,
                                          float frameDuration, bool looping)
{
    Animation anim;
    anim.m_looping = looping;

    for (int i = 0; i < frameCount; ++i)
    {
        AnimationFrame frame;
        frame.sourceRect = RECT{
            i * frameWidth, 0,
            i * frameWidth + frameWidth, frameHeight
        };
        frame.duration = frameDuration;
        anim.m_frames.push_back(frame);
        anim.m_totalDuration += frameDuration;
    }

    return anim;
}
