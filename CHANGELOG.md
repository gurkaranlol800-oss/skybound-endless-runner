# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-07-05

### Added

**Engine**
- Win32 windowing with message pump and fixed-size game window
- Direct3D 11 renderer: device/swap chain, alpha blending, vsync presentation, debug layer in debug builds
- Runtime HLSL shader compilation (`vs_5_0`/`ps_5_0`) with a sprite shader supporting UV sub-rects, horizontal flip, and tinting
- Texture loading via stb_image with point filtering for pixel art
- Sprite-sheet animation clips and a named-clip animation controller
- AABB physics with gravity, terminal velocity, and X/Y-separated swept tile collision
- Tilemap with solid and hazard tiles, endless rightward growth, and a text level-file loader
- Smoothed follow camera with level-bound clamping
- Fixed-timestep (60 Hz) game loop with accumulator and high-resolution timer
- Edge-triggered keyboard input (held vs. pressed-this-frame)
- Bitmap-digit HUD font with tinting
- Crash handler writing `crash_log.txt` on unhandled exceptions

**Game**
- Endless runner mode with chunked procedural generation, safe starting zone, and linear difficulty ramp (pits, spikes, platforms, enemies, coins)
- Player platforming with coyote time and jump buffering
- Patrol/chase enemy AI with ledge detection and stomp-to-kill combat
- Spike and pit hazards with game-over and instant restart
- Coin pickups with bob animation
- Parallax sky background
- HUD showing coins and distance
