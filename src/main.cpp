#include <Windows.h>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <random>
#include <type_traits>
#include "Window.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Sprite.h"
#include "Input.h"
#include "Animation.h"
#include "AnimationController.h"
#include "Tilemap.h"
#include "Player.h"
#include "Camera.h"
#include "Enemy.h"
#include "Pickup.h"
#include "Collision.h"
#include "EndlessGenerator.h"
#include "DigitFont.h"

namespace
{
    constexpr int kScreenWidth = 1280;
    constexpr int kScreenHeight = 720;
    constexpr int kLevelHeightTiles = kScreenHeight / Tilemap::kTileSize; // 18 - fixed; only width is endless
    // Gameplay updates run at a fixed rate independent of rendering/frame
    // rate, so movement speed etc. stays consistent even if frame timing
    // jitters. 1/60 second per update.
    constexpr double kFixedTimestep = 1.0 / 60.0;

    constexpr float kStompBounceSpeed = 600.0f;
    // How far the player's bottom edge may be above an enemy's top edge
    // and still count as landing on top of it (a stomp) rather than a
    // side/underneath hit - matches the classic Mario-style rule.
    constexpr float kStompTolerance = 20.0f;

    // The sky image scrolls slower than the foreground (a fraction of the
    // camera's movement) to fake distance/depth - the classic parallax
    // background trick.
    constexpr float kSkyParallaxFactor = 0.15f;

    // The generator keeps the map built out to this many tiles past the
    // right edge of the screen, so new content is never visibly "popping
    // in" right at the edge of view.
    constexpr int kGenerationBufferTiles = 20;

    // HUD layout - one shared white digit font recolored per-stat via
    // Sprite's tint parameter (colors are defined in wWinMain).
    constexpr float kHudPanelX = 16.0f, kHudPanelY = 16.0f;
    constexpr float kHudIconSize = 40.0f;
    constexpr float kHudDigitScale = 1.0f;

    // Falling further than this below the level's row range means the
    // player fell into a pit with nothing underneath - treated as death
    // rather than letting them fall forever off-screen.
    constexpr float kFallDeathY = static_cast<float>(kLevelHeightTiles) * Tilemap::kTileSize + 300.0f;

    enum class GameState { Playing, GameOver };
}

// High-resolution frame timer built on QueryPerformanceCounter, which is
// the standard Win32 way to measure elapsed time in fractional seconds.
class FrameTimer
{
public:
    FrameTimer()
    {
        QueryPerformanceFrequency(&m_frequency);
        QueryPerformanceCounter(&m_lastTime);
    }

    double TickSeconds()
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsed = static_cast<double>(now.QuadPart - m_lastTime.QuadPart) /
                          static_cast<double>(m_frequency.QuadPart);
        m_lastTime = now;
        return elapsed;
    }

private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_lastTime;
};

// Logs unhandled exceptions to crash_log.txt next to the executable -
// without this, a crash leaves no trace at all to diagnose after the
// fact (Windows only generates an Event Log entry for crashes in some
// configurations, and this app has no debugger attached in normal use).
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* info)
{
    FILE* f = nullptr;
    fopen_s(&f, "crash_log.txt", "a");
    if (f)
    {
        fprintf(f, "Unhandled exception: code=0x%08X address=0x%p\n",
                static_cast<unsigned int>(info->ExceptionRecord->ExceptionCode),
                info->ExceptionRecord->ExceptionAddress);
        fclose(f);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    SetUnhandledExceptionFilter(CrashHandler);

    Window window;
    if (!window.Create(hInstance, kScreenWidth, kScreenHeight, L"Game Engine - Endless Mode"))
        return -1;

    Renderer renderer;
    if (!renderer.Init(window.GetHandle(), kScreenWidth, kScreenHeight))
        return -1;

    Shader spriteShader;
    if (!spriteShader.LoadFromFile(renderer.GetDevice(), L"shaders/Sprite.hlsl"))
        return -1;

    // One sprite sheet texture per animation state - simplest placeholder
    // setup; a real asset pipeline might pack these into one atlas later.
    Texture idleTexture, runTexture, jumpTexture, tileTexture, hazardTexture, coinTexture, enemyTexture,
            skyTexture, gameOverTexture, hudPanelTexture, hudDistanceIconTexture;
    if (!idleTexture.LoadFromFile(renderer.GetDevice(), "assets/idle_sheet.png"))
        return -1;
    if (!runTexture.LoadFromFile(renderer.GetDevice(), "assets/run_sheet.png"))
        return -1;
    if (!jumpTexture.LoadFromFile(renderer.GetDevice(), "assets/jump_sheet.png"))
        return -1;
    if (!tileTexture.LoadFromFile(renderer.GetDevice(), "assets/tile.png"))
        return -1;
    if (!hazardTexture.LoadFromFile(renderer.GetDevice(), "assets/spike.png"))
        return -1;
    if (!coinTexture.LoadFromFile(renderer.GetDevice(), "assets/coin.png"))
        return -1;
    if (!enemyTexture.LoadFromFile(renderer.GetDevice(), "assets/enemy.png"))
        return -1;
    if (!skyTexture.LoadFromFile(renderer.GetDevice(), "assets/sky.png"))
        return -1;
    if (!gameOverTexture.LoadFromFile(renderer.GetDevice(), "assets/gameover.png"))
        return -1;
    if (!hudPanelTexture.LoadFromFile(renderer.GetDevice(), "assets/hud_panel.png"))
        return -1;
    if (!hudDistanceIconTexture.LoadFromFile(renderer.GetDevice(), "assets/hud_distance_icon.png"))
        return -1;

    DigitFont hudFont;
    if (!hudFont.Init(renderer.GetDevice(), "assets/hud_digits.png", 28, 36))
        return -1;
    const DirectX::XMFLOAT4 kCoinTint(1.0f, 0.84f, 0.25f, 1.0f);
    const DirectX::XMFLOAT4 kDistanceTint(0.55f, 0.92f, 1.0f, 1.0f);
    const DirectX::XMFLOAT4 kWhiteTint(1.0f, 1.0f, 1.0f, 1.0f);

    AnimationController animController;
    animController.AddClip("idle", &idleTexture,
        Animation::FromHorizontalStrip(64, 64, 4, 0.20f, /*looping=*/true));
    animController.AddClip("run", &runTexture,
        Animation::FromHorizontalStrip(64, 64, 6, 0.08f, /*looping=*/true));
    animController.AddClip("jump", &jumpTexture,
        Animation::FromHorizontalStrip(64, 64, 2, 0.15f, /*looping=*/false));
    animController.Play("idle");

    Sprite sprite;
    if (!sprite.Init(renderer.GetDevice()))
        return -1;

    Tilemap tilemap;
    EndlessGenerator generator;
    Player player;
    Camera camera;
    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;
    int coinsCollected = 0;
    int bestDistanceTiles = 0;
    GameState gameState = GameState::Playing;

    // Fixed spawn point, a few tiles above the ground - gravity settles
    // the player onto the (guaranteed-safe, hazard/enemy-free) starting
    // ground generated by EndlessGenerator, same as every other landing
    // in this engine.
    constexpr float kSpawnX = 2.0f * Tilemap::kTileSize;
    constexpr float kSpawnY = static_cast<float>((kLevelHeightTiles - 8) * Tilemap::kTileSize);

    auto SpawnFromTile = [](auto& container, float col, float row)
    {
        typename std::remove_reference_t<decltype(container)>::value_type obj;
        obj.Init(col * Tilemap::kTileSize, row * Tilemap::kTileSize);
        container.push_back(obj);
    };

    // (Re)starts a fresh endless run: a new random level, empty entity
    // lists, and the player back at the spawn point. Used both for the
    // very first run and every restart after Game Over.
    auto StartNewRun = [&]()
    {
        tilemap.InitEmpty(kLevelHeightTiles);
        generator.Init(kLevelHeightTiles, std::random_device{}());
        enemies.clear();
        pickups.clear();
        coinsCollected = 0;
        bestDistanceTiles = 0;

        int initialTargetCol = kScreenWidth / Tilemap::kTileSize + kGenerationBufferTiles;
        EndlessGenerator::ChunkSpawns initialSpawns = generator.GenerateAheadTo(tilemap, initialTargetCol);
        for (const auto& [col, row] : initialSpawns.enemies)
            SpawnFromTile(enemies, static_cast<float>(col), static_cast<float>(row));
        for (const auto& [col, row] : initialSpawns.pickups)
            SpawnFromTile(pickups, static_cast<float>(col), static_cast<float>(row));

        player.Init(kSpawnX, kSpawnY);
        camera.Init(tilemap.GetWidthTiles() * Tilemap::kTileSize, tilemap.GetHeightTiles() * Tilemap::kTileSize,
                    kScreenWidth, kScreenHeight);

        // Discard any jump-key press latched right before the restart
        // (e.g. held from the death that just ended the previous run) so
        // it can't fire an unwanted jump the instant the new run starts.
        Input::WasKeyPressed(VK_SPACE);
        Input::WasKeyPressed(VK_UP);
        Input::WasKeyPressed('W');

        gameState = GameState::Playing;
    };

    StartNewRun();

    FrameTimer timer;
    double accumulator = 0.0;

    bool running = true;
    while (running)
    {
        double frameSeconds = timer.TickSeconds();
        // Clamp to avoid a huge catch-up burst after a debugger breakpoint
        // or window drag stalls the loop.
        if (frameSeconds > 0.25)
            frameSeconds = 0.25;
        accumulator += frameSeconds;

        running = window.ProcessMessages();

        // Run gameplay logic in fixed-size steps regardless of how much
        // real time actually elapsed this frame.
        while (accumulator >= kFixedTimestep)
        {
            float dt = static_cast<float>(kFixedTimestep);

            if (Input::IsKeyDown(VK_ESCAPE))
                running = false;

            if (gameState == GameState::GameOver)
            {
                if (Input::WasKeyPressed('R'))
                    StartNewRun();

                accumulator -= kFixedTimestep;
                continue;
            }

            bool moveLeft = Input::IsKeyDown('A') || Input::IsKeyDown(VK_LEFT);
            bool moveRight = Input::IsKeyDown('D') || Input::IsKeyDown(VK_RIGHT);
            bool jumpPressed = Input::WasKeyPressed(VK_SPACE) ||
                                Input::WasKeyPressed(VK_UP) ||
                                Input::WasKeyPressed('W');
            bool isMoving = moveLeft || moveRight;

            player.Update(dt, moveLeft, moveRight, jumpPressed, tilemap);
            bestDistanceTiles = std::max(bestDistanceTiles,
                                          static_cast<int>(player.GetX() / Tilemap::kTileSize));

            // Keep the map generated well past the camera's right edge,
            // and let the camera's right-scroll bound grow to match.
            int targetCol = static_cast<int>(camera.GetX()) / Tilemap::kTileSize +
                             kScreenWidth / Tilemap::kTileSize + kGenerationBufferTiles;
            EndlessGenerator::ChunkSpawns newSpawns = generator.GenerateAheadTo(tilemap, targetCol);
            for (const auto& [col, row] : newSpawns.enemies)
                SpawnFromTile(enemies, static_cast<float>(col), static_cast<float>(row));
            for (const auto& [col, row] : newSpawns.pickups)
                SpawnFromTile(pickups, static_cast<float>(col), static_cast<float>(row));
            camera.SetLevelWidth(tilemap.GetWidthTiles() * Tilemap::kTileSize);

            float playerCenterX = player.GetX() + Player::kWidth * 0.5f;
            float playerCenterY = player.GetY() + Player::kHeight * 0.5f;
            for (Enemy& enemy : enemies)
                enemy.Update(dt, tilemap, playerCenterX, playerCenterY);

            for (Pickup& pickup : pickups)
                pickup.Update(dt);

            // All gameplay overlap checks use the tighter collision box,
            // not the visual sprite box - see Player::kCollisionWidth for
            // why (the sprite frame has a lot of transparent padding, so
            // using its full size as the hitbox made touches feel late
            // or "through" empty-looking space).
            float pcx = player.GetCollisionX();
            float pcy = player.GetCollisionY();

            // Player vs pickups: simple AABB overlap, first touch wins.
            for (Pickup& pickup : pickups)
            {
                if (pickup.IsCollected())
                    continue;

                if (AABBOverlap(pcx, pcy, Player::kCollisionWidth, Player::kCollisionHeight,
                                 pickup.GetX(), pickup.GetY(), Pickup::kSize, Pickup::kSize))
                {
                    pickup.Collect();
                    ++coinsCollected;
                }
            }

            bool died = false;

            // Player vs enemies: landing on top stomps the enemy and
            // bounces the player; touching from any other angle kills
            // the player instead.
            for (Enemy& enemy : enemies)
            {
                if (!enemy.IsAlive())
                    continue;

                if (!AABBOverlap(pcx, pcy, Player::kCollisionWidth, Player::kCollisionHeight,
                                  enemy.GetCollisionX(), enemy.GetCollisionY(),
                                  Enemy::kCollisionWidth, Enemy::kCollisionHeight))
                    continue;

                bool isStomp = player.GetVY() > 0.0f &&
                               (pcy + Player::kCollisionHeight) - enemy.GetCollisionY() < kStompTolerance;
                if (isStomp)
                {
                    enemy.Kill();
                    player.Bounce(kStompBounceSpeed);
                }
                else
                {
                    died = true;
                }
            }

            if (tilemap.OverlapsHazard(pcx, pcy, Player::kCollisionWidth, Player::kCollisionHeight))
                died = true;

            // Fell into a pit with nothing below it.
            if (pcy > kFallDeathY)
                died = true;

            if (died)
            {
                gameState = GameState::GameOver;
                accumulator -= kFixedTimestep;
                continue;
            }

            camera.Follow(playerCenterX, playerCenterY, dt);

            // Animation state follows physics directly: airborne (jumping
            // or falling) always shows the jump pose; otherwise idle/run
            // follows whether a movement key is held.
            animController.Play(!player.IsGrounded() ? "jump" : (isMoving ? "run" : "idle"));
            animController.Update(dt);

            accumulator -= kFixedTimestep;
        }

        wchar_t title[160];
        if (gameState == GameState::Playing)
        {
            swprintf_s(title, L"Game Engine - Endless Mode | Coins: %d  Distance: %d",
                       coinsCollected, bestDistanceTiles);
        }
        else
        {
            swprintf_s(title, L"Game Engine - GAME OVER | Coins: %d  Distance: %d  (Press R to restart)",
                       coinsCollected, bestDistanceTiles);
        }
        SetWindowTextW(window.GetHandle(), title);

        renderer.BeginFrame(0.10f, 0.10f, 0.15f, 1.0f);

        // Sky scrolls slower than the foreground (parallax) and is sized
        // wider than the screen so it never runs out of coverage even at
        // the level's maximum camera scroll. Clamped defensively so a gap
        // can never appear on the right edge no matter how far the
        // endless generator has scrolled the camera.
        float maxSkyOffsetX = static_cast<float>(skyTexture.GetWidth() - kScreenWidth);
        float skyOffsetX = std::min(camera.GetX() * kSkyParallaxFactor, maxSkyOffsetX);
        RECT skyFrame = { 0, 0, skyTexture.GetWidth(), skyTexture.GetHeight() };
        sprite.Draw(renderer.GetContext(), spriteShader, skyTexture,
                    -skyOffsetX, 0.0f, static_cast<float>(skyTexture.GetWidth()), static_cast<float>(kScreenHeight),
                    kScreenWidth, kScreenHeight, skyFrame);

        tilemap.Draw(renderer.GetContext(), spriteShader, tileTexture, hazardTexture, sprite,
                     camera.GetX(), camera.GetY(), kScreenWidth, kScreenHeight);

        RECT coinFrame = { 0, 0, coinTexture.GetWidth(), coinTexture.GetHeight() };
        for (const Pickup& pickup : pickups)
        {
            if (pickup.IsCollected())
                continue;
            sprite.Draw(renderer.GetContext(), spriteShader, coinTexture,
                        pickup.GetX() - camera.GetX(), pickup.GetY() - camera.GetY(),
                        Pickup::kSize, Pickup::kSize, kScreenWidth, kScreenHeight, coinFrame);
        }

        RECT enemyFrame = { 0, 0, enemyTexture.GetWidth(), enemyTexture.GetHeight() };
        for (const Enemy& enemy : enemies)
        {
            if (!enemy.IsAlive())
                continue;
            sprite.Draw(renderer.GetContext(), spriteShader, enemyTexture,
                        enemy.GetX() - camera.GetX(), enemy.GetY() - camera.GetY(),
                        Enemy::kWidth, Enemy::kHeight, kScreenWidth, kScreenHeight, enemyFrame,
                        enemy.IsFacingLeft());
        }

        sprite.Draw(renderer.GetContext(), spriteShader, *animController.GetCurrentTexture(),
                    player.GetX() - camera.GetX(), player.GetY() - camera.GetY(),
                    Player::kWidth, Player::kHeight,
                    kScreenWidth, kScreenHeight, animController.GetCurrentFrameRect(),
                    player.IsFacingLeft());

        if (gameState == GameState::GameOver)
        {
            RECT gameOverFrame = { 0, 0, gameOverTexture.GetWidth(), gameOverTexture.GetHeight() };
            sprite.Draw(renderer.GetContext(), spriteShader, gameOverTexture,
                        0.0f, 0.0f, static_cast<float>(kScreenWidth), static_cast<float>(kScreenHeight),
                        kScreenWidth, kScreenHeight, gameOverFrame);
        }

        // HUD: fixed in screen space (no camera offset), drawn last so
        // it's always on top - including over the Game Over overlay, so
        // the final tally stays readable.
        RECT hudPanelFrame = { 0, 0, hudPanelTexture.GetWidth(), hudPanelTexture.GetHeight() };
        sprite.Draw(renderer.GetContext(), spriteShader, hudPanelTexture,
                    kHudPanelX, kHudPanelY,
                    static_cast<float>(hudPanelTexture.GetWidth()), static_cast<float>(hudPanelTexture.GetHeight()),
                    kScreenWidth, kScreenHeight, hudPanelFrame);

        float iconX = kHudPanelX + 16.0f;
        float coinIconY = kHudPanelY + 12.0f;
        float distIconY = coinIconY + kHudIconSize + 10.0f;
        float textX = iconX + kHudIconSize + 12.0f;

        RECT coinIconFrame = { 0, 0, coinTexture.GetWidth(), coinTexture.GetHeight() };
        sprite.Draw(renderer.GetContext(), spriteShader, coinTexture,
                    iconX, coinIconY, kHudIconSize, kHudIconSize,
                    kScreenWidth, kScreenHeight, coinIconFrame, false, kWhiteTint);
        hudFont.DrawNumber(renderer.GetContext(), spriteShader, sprite, coinsCollected,
                           textX, coinIconY, kHudDigitScale, kScreenWidth, kScreenHeight, kCoinTint);

        RECT distIconFrame = { 0, 0, hudDistanceIconTexture.GetWidth(), hudDistanceIconTexture.GetHeight() };
        sprite.Draw(renderer.GetContext(), spriteShader, hudDistanceIconTexture,
                    iconX, distIconY, kHudIconSize, kHudIconSize,
                    kScreenWidth, kScreenHeight, distIconFrame, false, kWhiteTint);
        hudFont.DrawNumber(renderer.GetContext(), spriteShader, sprite, bestDistanceTiles,
                           textX, distIconY, kHudDigitScale, kScreenWidth, kScreenHeight, kDistanceTint);

        renderer.EndFrame();
    }

    return 0;
}
