#pragma once
#include <string>
#include <unordered_map>
#include "Animation.h"
#include "Texture.h"

// A minimal animation state machine: holds a set of named clips (each with
// its own texture, since our placeholder art uses one sheet per state),
// tracks which one is currently playing, and advances playback time.
// Gameplay code decides *when* to switch clips (e.g. "moving -> run") by
// calling Play(); this class only owns *how* a clip plays back.
class AnimationController
{
public:
    void AddClip(const std::string& name, Texture* texture, Animation animation);

    // Switches to the named clip, restarting it from frame 0. Calling
    // Play() with the clip that's already active is a no-op, so looping
    // gameplay code (e.g. "still moving -> play run") doesn't stutter by
    // resetting to frame 0 every update.
    void Play(const std::string& name);

    void Update(float dt);

    // True once a non-looping clip (e.g. "jump") has reached its last
    // frame and stopped advancing - lets gameplay code know when to
    // transition back to idle/run.
    bool IsFinished() const { return m_finished; }

    const std::string& GetCurrentClipName() const { return m_currentClipName; }
    Texture* GetCurrentTexture() const;
    RECT GetCurrentFrameRect() const;

private:
    struct Clip
    {
        Texture* texture;
        Animation animation;
    };

    std::unordered_map<std::string, Clip> m_clips;
    std::string m_currentClipName;
    float m_elapsed = 0.0f;
    bool m_finished = false;
};
