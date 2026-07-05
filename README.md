# Skybound — Endless Runner

> A 2D endless-runner platformer and custom game engine written from scratch in **C++17** with **Win32** and **Direct3D 11** — no frameworks, no engines, no external dependencies beyond the Windows SDK and a single vendored header.

Sprint through a procedurally generated world that gets harder the farther you run. Stomp enemies, dodge spikes, clear pits, and collect coins — all rendered by a hand-built D3D11 sprite pipeline running on a fixed-timestep simulation.

## 🎮 Download & Play

### [⬇ Download for Windows (zip, 204 KB)](https://github.com/gurkaranlol800-oss/skybound-endless-runner/releases/download/v1.0.0/Skybound-v1.0.0-win64.zip)

**No build tools, nothing to install** — download the zip above, extract it, and double-click `GameEngine.exe`. Works on any 64-bit Windows 10/11 PC with a Direct3D 11-capable GPU. All versions are on the [releases page](https://github.com/gurkaranlol800-oss/skybound-endless-runner/releases/latest).

> If Windows SmartScreen warns about an unrecognized app, click **More info → Run anyway** (the build isn't code-signed; full source is right here in this repo).

## Overview

This project is two things in one repository:

1. **A minimal 2D game engine** — windowing, renderer, shader pipeline, texture loading, sprite drawing, sprite-sheet animation, AABB physics, tilemap collision, camera, input, and a bitmap-font HUD, each implemented as a small focused module.
2. **A complete game built on it** — an endless runner with procedural level generation, enemy AI, pickups, hazards, difficulty scaling, and a game-over/restart loop.

Everything is written directly against the Win32 API and Direct3D 11. The goal was to understand how a game engine actually works underneath — the swap chain, the constant buffers, the fixed-timestep accumulator — rather than have a framework do it.

## Features

### Engine
- **Direct3D 11 renderer** — device/swap-chain setup, alpha blending, vsync presentation, and a debug layer in debug builds
- **Runtime HLSL shader compilation** (`vs_5_0` / `ps_5_0`) with a single sprite shader supporting UV sub-rects, horizontal flip, and RGBA tinting
- **Texture loading via stb_image** with point filtering for crisp pixel art
- **Sprite-sheet animation system** — frame strips with per-frame timing, plus a named-clip controller (idle / run / jump) handling looping and one-shot clips
- **AABB physics** — gravity, terminal velocity, and swept tile collision resolved on X and Y axes separately, with grounded and wall-hit detection
- **Tilemap** with solid and hazard tiles (40 px grid) that can grow endlessly to the right, plus a text-based level file loader (`assets/levels/level1.txt`) for hand-authored levels
- **Smoothed follow camera** with exponential smoothing and level-bound clamping
- **Fixed-timestep game loop** — 60 Hz simulation with an accumulator (0.25 s spiral-of-death clamp) driven by a high-resolution `QueryPerformanceCounter` timer
- **Edge-triggered keyboard input** — distinguishes "held" from "pressed this frame"
- **Bitmap-digit HUD font** with per-stat tinting
- **Crash handler** that writes a `crash_log.txt` next to the executable on unhandled exceptions

### Game
- **Procedural endless generation** — 16-tile chunks with jump-clearable pits, spikes, reachable platforms topped with coins, and enemy spawns; a safe starting zone; difficulty ramps linearly over the first 500 tiles
- **Tight platforming feel** — coyote time (0.10 s), jump buffering (0.12 s), and a collision box trimmed smaller than the sprite art
- **Enemy AI** — ground patrol with ledge detection, switching to a faster chase when the player is spotted
- **Stomp combat** — bounce off enemies from above; touching them from the side is fatal
- **Hazards** — spikes and bottomless pits
- **Coin pickups** with a sine-wave bob animation
- **Parallax sky background**
- **Live HUD** showing coins collected and distance travelled, plus a game-over screen with instant restart

## Technologies

| | |
|---|---|
| Language | C++17 |
| Graphics | Direct3D 11, HLSL (compiled at runtime via `d3dcompiler`) |
| Platform | Win32 API (windowing, input, timing) |
| Build | CMake ≥ 3.20 → Visual Studio / MSVC |
| Image loading | [stb_image](https://github.com/nothings/stb) (vendored single header) |

No other dependencies — the engine links only `d3d11`, `dxgi`, and `d3dcompiler` from the Windows SDK.

## Architecture

Each engine concern lives in its own small module under [src/](src/):

| Module | Responsibility |
|---|---|
| `main.cpp` | Entry point, fixed-timestep game loop, game states (Playing / GameOver), gameplay wiring |
| `Window` | Win32 window creation and message pump |
| `Renderer` | D3D11 device, swap chain, render target, blending, frame begin/end |
| `Shader` | Runtime HLSL compilation, input layout |
| `Texture` | Image loading (stb_image) → GPU texture + sampler |
| `Sprite` | Textured quad drawing: MVP transform, UV sub-rect, flip, tint |
| `Animation` / `AnimationController` | Frame-strip clips and named-clip playback |
| `Input` | Keyboard state with held vs. pressed-this-frame semantics |
| `PhysicsBody` | Gravity, acceleration/friction, swept AABB-vs-tilemap resolution |
| `Collision` | Entity-vs-entity AABB overlap helper |
| `Tilemap` | Solid/hazard tile grid, level-file loading, culled drawing |
| `Camera` | Smoothed follow with level-bound clamping |
| `Player` | Movement, jumping (coyote time + buffering), stomp bounce |
| `Enemy` | Patrol/chase AI with ledge probing |
| `Pickup` | Bobbing collectible coins |
| `EndlessGenerator` | Chunked procedural level generation with difficulty ramp |
| `DigitFont` | Bitmap number rendering for the HUD |

**Frame flow:** the loop polls Win32 messages, accumulates real elapsed time, and steps the simulation at a fixed 60 Hz (input → player/enemy/pickup update → physics → collision/gameplay events → camera → endless generation ahead of the camera). Rendering then draws the parallax sky, tilemap, entities, and HUD once per frame, and presents with vsync.

## Controls

| Action | Keys |
|---|---|
| Move left / right | `A` / `D` or `←` / `→` |
| Jump | `Space`, `W`, or `↑` |
| Restart (after game over) | `R` |

## Folder Structure

```
game-engine/
├── CMakeLists.txt          # Build configuration
├── src/                    # All engine + game source (C++17)
├── shaders/
│   └── Sprite.hlsl         # Sprite vertex + pixel shader
├── assets/                 # Sprites, sprite sheets, HUD art
│   └── levels/level1.txt   # Text-format authored level (for the level loader)
├── third_party/
│   └── stb_image.h         # Vendored image-loading header
└── docs/
    └── screenshots/        # Screenshots used in this README
```

## Building

**Requirements:** Windows 10/11 · Visual Studio 2019/2022 with the *Desktop development with C++* workload (includes the Windows SDK) · CMake ≥ 3.20 · a Direct3D 11-capable GPU.

```powershell
git clone <this-repo-url>
cd game-engine
cmake -B build
cmake --build build --config Release
```

Run the game:

```powershell
build\Release\GameEngine.exe
```

A post-build step copies `assets/` and `shaders/` next to the executable, so it can be launched directly from the build output folder.

## Screenshots

> Screenshots are coming soon — they will live in [docs/screenshots/](docs/screenshots/).

<!-- Add captures here, e.g.:
![Gameplay](docs/screenshots/gameplay.png)
![Game over](docs/screenshots/gameover.png)
-->

## Roadmap

- [ ] Sound effects and music (XAudio2)
- [ ] Sprite batching / instancing (currently one draw call per sprite)
- [ ] Main menu and pause state
- [ ] High-score persistence between runs
- [ ] Gamepad support
- [ ] Authored-level campaign mode using the existing text level loader
- [ ] Application icon and polished window branding

## License

This project is licensed under the [MIT License](LICENSE).

[stb_image](https://github.com/nothings/stb) (in `third_party/`) is by Sean Barrett and is available under the MIT License / public domain — see [third_party/README.md](third_party/README.md).
