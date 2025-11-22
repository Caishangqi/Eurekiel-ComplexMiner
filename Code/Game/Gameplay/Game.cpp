#include <Game/Gameplay/Game.hpp>

#include "../Framework/App.hpp"
#include "../GameCommon.hpp"
#include "Generator/SimpleMinerGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/SmoothNoise.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Resource system integration
#include "Engine/Resource/ResourceSubsystem.hpp"
#include "Engine/Resource/ResourceCommon.hpp"
#include "Engine/Core/Yaml.hpp"
#include "Engine/Core/LogCategory/PredefinedCategories.hpp"
#include "Engine/Core/Schedule/ScheduleSubsystem.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Engine/Voxel/Chunk/Chunk.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Voxel/Builtin/DefaultBlock.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Framework/DummyTask.hpp"
#include "Game/Framework/GUISubsystem.hpp"
#include "gui/GUIDebugLight.hpp"
#include "gui/GUIProfiler.hpp"
#include "Game/Framework/World/WorldConstant.hpp"
#include "Player/Player.hpp"


Game::Game()
{
    /// Rasterize
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

    /// Spaces
    IntVec2 clientDimension = g_theWindow->GetClientDimensions();
    m_screenSpace.m_mins    = Vec2::ZERO;
    m_screenSpace.m_maxs    = Vec2(clientDimension);

    m_worldSpace.m_mins = Vec2::ZERO;
    m_worldSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("worldSizeX", 200.f), g_gameConfigBlackboard.GetValue("worldSizeY", 100.f));

    /// Cameras
    m_screenCamera         = new Camera();
    m_screenCamera->m_mode = eMode_Orthographic;
    m_screenCamera->SetOrthographicView(Vec2::ZERO, m_screenSpace.m_maxs);
    ///

    /// Clock
    m_clock      = new Clock(Clock::GetSystemClock());
    m_worldClock = new Clock(Clock::GetSystemClock()); // 世界时间时钟
    ///

    /// Player
    m_player                = new Player(this);
    m_player->m_position    = Vec3(0, 0, 128);
    m_player->m_orientation = EulerAngles(-45, 30, 0);
    /// 

    /// Game State
    g_theInput->SetCursorMode(CursorMode::POINTER);

    /// Block Registration Phase - MUST happen before World creation
    RegisterBlocks();

    /// World Creation with RAII constructor
    using namespace enigma::voxel;

    auto generator     = std::make_unique<SimpleMinerGenerator>();
    m_world            = std::make_unique<World>("world", 6693073380, std::move(generator));
    int renderDistance = settings.GetInt("video.simulationDistance", 24);
    m_world->SetChunkActivationRange(renderDistance);
    LogInfo(LogGame, "Render distance configured: %d chunks (using independent generators per chunk)", renderDistance);

    /// Resource preload
    m_worldShader = g_theRenderer->CreateOrGetShader(".enigma/data/Shaders/World");
    m_worldCBO    = g_theRenderer->CreateConstantBuffer(sizeof(WorldConstant));
}

Game::~Game()
{
    // Save and close world before cleanup
    if (m_world)
    {
        LogInfo(LogGame, "Saving world before game shutdown...");
        m_world->SaveWorld();

        // ===== Phase 5: Graceful Shutdown =====
        LogInfo(LogGame, "Initiating graceful shutdown...");
        m_world->PrepareShutdown(); // Stop new tasks
        m_world->WaitForPendingTasks(); // Wait for completion

        LogInfo(LogGame, "Closing world...");
        m_world->CloseWorld();
        m_world.reset();
    }

    POINTER_SAFE_DELETE(m_worldCBO)
    POINTER_SAFE_DELETE(m_player)
    POINTER_SAFE_DELETE(m_screenCamera)
    POINTER_SAFE_DELETE(m_worldCamera)
    POINTER_SAFE_DELETE(m_worldClock)
    POINTER_SAFE_DELETE(m_clock)

    g_theEventSystem->FireEvent("Event.Game.GameInstanceRemove");
}


void Game::Render() const
{
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetBlendMode(blend_mode::ALPHA);
    g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(depth_mode::READ_WRITE_LESS_EQUAL);
    if (!m_isInMainMenu)
    {
        m_player->Render();
        RenderWorld();
        DebugRenderScreen(*g_theGame->m_screenCamera);
        DebugRenderWorld(*g_theGame->m_player->m_camera);
    }


    //======================================================================= End of World Render =======================================================================
    // Second render screen camera
    g_theRenderer->BeginCamera(*m_screenCamera);
    /// Display Only
#ifdef COSMIC
    if (m_isInMainMenu)
    {
        g_theRenderer->ClearScreen(g_theApp->m_backgroundColor);
        g_theRenderer->BindTexture(nullptr);
        DebugDrawRing(m_screenSpace.m_maxs / 2, m_currentIconCircleThickness, m_currentIconCircleThickness / 10, Rgba8::WHITE);
    }
#endif
    // UI render
    g_theRenderer->EndCamera(*m_screenCamera);
    //======================================================================= End of Screen Render =======================================================================
    /// 
}


void Game::UpdateCameras(float deltaTime)
{
    m_screenCamera->Update(deltaTime);
}

void Game::Update()
{
    if (m_isInMainMenu)
    {
        g_theInput->SetCursorMode(CursorMode::POINTER);
    }

    if (m_isGameStart)
    {
        UpdateWorld();
    }

    /// Player
    m_player->Update(Clock::GetSystemClock().GetDeltaSeconds());
    ///

    /// Cube
    float brightnessFactor = CycleValue(m_clock->GetTotalSeconds(), 1.f);
    auto  color            = Rgba8(
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        255);
    /// 


    /// Debug Only
    std::string debugGameState = Stringf("Time: %.2f FPS: %.1f Scale: %.2f",
                                         m_clock->GetTotalSeconds(),
                                         m_clock->GetFrameRate(),
                                         m_clock->GetTimeScale()
    );
    DebugAddScreenText(debugGameState, m_screenSpace, 14, 0);
    DebugAddMessage(Stringf("Player position: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z), 0);

    // Display current camera mode in top right corner
    std::string cameraModeText    = Stringf("Camera: [ %s ]", CameraModeToString(m_player->GetCameraMode()));
    AABB2       cameraModeTextBox = m_screenSpace.GetPadded(Vec4(0, 0, 0, -16));
    DebugAddScreenText(cameraModeText, cameraModeTextBox, 14.0f, 0.0f, Rgba8::ORANGE, Rgba8::ORANGE, Vec2(1, 1.f));

    // Chunk management debug information (Assignment 02 requirement)
    if (m_world)
    {
        // Calculate player's current chunk coordinates using proper constants
        int32_t playerChunkX = static_cast<int32_t>(std::floor(m_player->m_position.x / enigma::voxel::Chunk::CHUNK_SIZE_X));
        int32_t playerChunkY = static_cast<int32_t>(std::floor(m_player->m_position.y / enigma::voxel::Chunk::CHUNK_SIZE_Y));

        UNUSED(playerChunkX)
        UNUSED(playerChunkY)

        // Display chunk management stats
        //DebugAddMessage(Stringf("Chunk: (%d, %d) | Loaded: %zu chunks",playerChunkX, playerChunkY, m_world->GetLoadedChunkCount()), 1);

        // Show activation/deactivation ranges
        //DebugAddMessage(Stringf("Activation Range: %d chunks",settings.GetInt("video.simulationDistance")), 2);
    }

    /// Display Only
#ifdef COSMIC
    m_counter++;
    m_currentIconCircleThickness = FluctuateValue(m_iconCircleRadius, 50.f, 0.02f, static_cast<float>(m_counter));
#endif
    float deltaTime = m_clock->GetDeltaSeconds();
    UpdateCameras(deltaTime);
    HandleMouseEvent(deltaTime);
    HandleKeyBoardEvent(deltaTime);
}

void Game::UpdateWorld()
{
    if (m_world)
    {
        // Sync player position to world and chunk manager for intelligent chunk loading
        if (m_player)
        {
            m_world->SetPlayerPosition(m_player->m_position);
        }

        m_world->Update(m_clock->GetDeltaSeconds());

        // [NEW] Phase 12: Unified lightning and glowstone update
        UpdateLightningAndGlow();
    }
}

float Game::GetTimeOfDay() const
{
    // Base ratio: 500:1 (world time: real time)
    // 1 real second = 500 world seconds = 500/86400 world days ≈ 0.00579 world days
    constexpr float WORLD_TIME_SCALE = 500.0f / (60.0f * 60.0f * 24.0f); // 500/86400 ≈ 0.00579

    float totalSeconds  = m_worldClock->GetTotalSeconds();
    float worldTimeDays = totalSeconds * WORLD_TIME_SCALE;

    // Returns the time of day [0.0, 1.0)
    // 0.0 = midnight, 0.25 = dawn 6am, 0.5 = noon, 0.75 = dusk 6pm
    return fmodf(worldTimeDays, 1.0f);
}

Rgba8 Game::CalculateSkyColor(float timeOfDay) const
{
    //Define sky color constants
    Rgba8 nightSky(20, 20, 40); // dark blue (night)
    Rgba8 noonSky(200, 230, 255); // bright blue (noon)

    // Daytime period (6am-6pm, timeOfDay: 0.25-0.75)
    if (timeOfDay >= 0.25f && timeOfDay <= 0.75f)
    {
        // Calculate progress during the day [0.0, 1.0]
        // 0.0 = 6am, 0.5 = noon, 1.0 = 6pm
        float dayProgress = (timeOfDay - 0.25f) / 0.5f;

        // Calculate the noon factor [0.0, 1.0]
        // Use trigonometric functions to reach the maximum value of 1.0 at noon
        // 6am=0.0, noon=1.0, 6pm=0.0
        float noonFactor = 1.0f - fabsf(dayProgress - 0.5f) * 2.0f;

        // Use Engine's Interpolate function for color interpolation
        return Interpolate(nightSky, noonSky, noonFactor);
    }

    // Night time period (6pm-6am, timeOfDay: 0.75-1.0 or 0.0-0.25)
    // keep constant dark blue
    return nightSky;
}

Rgba8 Game::CalculateOutdoorLightColor(float timeOfDay) const
{
    //Define outdoor light color constants
    Rgba8 midnightLight(40, 50, 80); // Dark bluish gray (midnight light)
    Rgba8 dayLight(255, 255, 255); // pure white (daylight)

    // Daytime period (6am-6pm, timeOfDay: 0.25-0.75)
    if (timeOfDay >= 0.25f && timeOfDay <= 0.75f)
    {
        // Calculate progress during the day [0.0, 1.0]
        // 0.0 = 6am, 0.5 = noon, 1.0 = 6pm
        float dayProgress = (timeOfDay - 0.25f) / 0.5f;

        // Calculate the noon factor [0.0, 1.0]
        // Use trigonometric functions to reach the maximum value of 1.0 at noon
        // 6am=0.0, noon=1.0, 6pm=0.0
        float noonFactor = 1.0f - fabsf(dayProgress - 0.5f) * 2.0f;

        // Use Engine's Interpolate function for color interpolation
        return Interpolate(midnightLight, dayLight, noonFactor);
    }

    // Night time period (6pm-6am, timeOfDay: 0.75-1.0 or 0.0-0.25)
    // Keep constant dark bluish gray
    return midnightLight;
}

void Game::UpdateLightning()
{
    // [FIX] Phase 12: Lightning effect - Assignment 05 compliant (instant flash)
    float worldTime = m_worldClock->GetTotalSeconds();

    // [STEP 1] Calculate Perlin noise (9 octaves, scale 200)
    float lightningPerlin = Compute1dPerlinNoise(worldTime, 1.0f, 9, 0.5f, 2.0f, true, 0);

    // [STEP 2] Range-map [0.6, 0.9] → [0, 1] and clamp
    // Note: Perlin outputs [-1, 1], only values in [0.6, 0.9] trigger lightning
    m_lightningStrength = RangeMap(lightningPerlin, 0.6f, 0.9f, 0.0f, 1.0f);
    m_lightningStrength = GetClamped(m_lightningStrength, 0.0f, 1.0f);

    // [STEP 3] Calculate base colors (from day/night cycle) - MUST recalculate each frame
    float timeOfDay             = GetTimeOfDay();
    Rgba8 baseSkyColor          = CalculateSkyColor(timeOfDay);
    Rgba8 baseOutdoorLightColor = CalculateOutdoorLightColor(timeOfDay);

    // [STEP 4] Lerp toward white based on lightningStrength (Assignment 05: instant flash)
    // This is NOT a gradual transition - lightningStrength changes instantly with Perlin noise
    Rgba8 whiteLightning(255, 255, 255);
    m_skyColor          = Interpolate(baseSkyColor, whiteLightning, m_lightningStrength);
    m_outdoorLightColor = Interpolate(baseOutdoorLightColor, whiteLightning, m_lightningStrength);
}

void Game::UpdateGlowstoneFlicker()
{
    // [NEW] Phase 12: Glowstone flicker effect (9 octaves, scale 500)
    float worldTime  = m_worldClock->GetTotalSeconds();
    float glowPerlin = Compute1dPerlinNoise(worldTime, 0.8f, 9, 0.5f, 2.0f, true, 0);

    // Range-map [-1, 1] → [0.8, 1.0] (no clamp needed, glowPerlin already in range)
    m_glowstoneFlickerStrength = RangeMap(glowPerlin, -1.0f, 1.0f, 0.8f, 1.0f);

    // Apply flicker to indoor light color (255, 230, 204) using Rgba8 operator*
    Rgba8 baseIndoorLight(255, 230, 204);
    m_indoorLightColor = baseIndoorLight * m_glowstoneFlickerStrength;
}

void Game::UpdateLightningAndGlow()
{
    // [NEW] Phase 12: Unified update for lightning and glowstone effects

    // Update lightning effect (affects sky and outdoor light)
    UpdateLightning();

    // Update glowstone flicker effect (affects indoor light)
    UpdateGlowstoneFlicker();
}

void Game::RenderWorld() const
{
    if (m_world)
    {
        // [REMOVED] Phase 12: DO NOT recalculate colors here - they are set by UpdateLightning()
        // m_skyColor and m_outdoorLightColor are already calculated in UpdateLightningAndGlow()
        // which includes base day/night colors + lightning effect

        // Set to frame clear color (sky background)
        g_theRenderer->ClearScreen(m_skyColor);

        // [STEP 2] Prepare WorldConstants data
        WorldConstant worldConstants = {};

        //Camera position (obtained from player's camera)
        Vec3 cameraPos                = m_player->m_camera->GetPosition();
        worldConstants.CameraPosition = Vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);

        //Light color
        // [MODIFIED] Phase 12: Indoor light affected by glowstone flicker
        worldConstants.IndoorLightColor = Vec4(
            (float)m_indoorLightColor.r / 255.0f,
            (float)m_indoorLightColor.g / 255.0f,
            (float)m_indoorLightColor.b / 255.0f,
            1.0f
        );

        // Outdoor light dynamic interpolation based on TimeOfDay
        worldConstants.OutdoorLightColor = Vec4(
            (float)m_outdoorLightColor.r / 255.0f,
            (float)m_outdoorLightColor.g / 255.0f,
            (float)m_outdoorLightColor.b / 255.0f,
            1.0f
        );

        // [NEW] 天空颜色 - 基于TimeOfDay动态插值
        worldConstants.SkyColor = Vec4(
            (float)m_skyColor.r / 255.0f,
            (float)m_skyColor.g / 255.0f,
            (float)m_skyColor.b / 255.0f,
            1.0f
        );

        // [FIX] Fog distance - calculated according to Assignment 05 requirements
        // FogFar = activation_range - (2 * chunk_size)
        // FogNear = FogFar * 0.9 (transition starts at 90%)
        float activationRange          = 12.0f * 16.0f * 2; // 384格 (12区块 * 16格/区块 * 2)
        worldConstants.FogFarDistance  = activationRange - (2.0f * 16.0f); // 384 - 32 = 352格
        worldConstants.FogNearDistance = worldConstants.FogFarDistance * 0.9f; // 352 * 0.9 = 316.8格

        // [STEP 3] Upload to GPU (must be before BindShader)
        g_theRenderer->CopyCPUToGPU(&worldConstants, sizeof(WorldConstant), m_worldCBO);
        g_theRenderer->BindConstantBuffer(4, m_worldCBO); // register(b4)

        // [STEP 4] Bind Shader and render
        g_theRenderer->BindShader(m_worldShader);
        m_world->Render(g_theRenderer);
        g_theRenderer->BindShader(nullptr);
    }
}


void Game::HandleKeyBoardEvent(float deltaTime)
{
    UNUSED(deltaTime)
    const XboxController& controller = g_theInput->GetController(0);

    // [NEW] Y key accelerates world time (50x acceleration)
    if (m_worldClock)
    {
        float acceleration = g_theInput->IsKeyDown('Y') ? 50.0f : 1.0f;
        m_worldClock->SetTimeScale(acceleration);
    }


    if (m_isInMainMenu)
    {
        bool spaceBarPressed = g_theInput->WasKeyJustPressed(32);
        bool NKeyPressed     = g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(XBOX_BUTTON_A) || controller.WasButtonJustPressed(XBOX_BUTTON_START);
        if (spaceBarPressed || NKeyPressed)
        {
            StartGame();
        }
    }

    // Disable Chunk Debug Rendering

    // F3 + G Enable Chunk Debug border
    if (g_theInput->IsKeyDown(0x72) && g_theInput->WasKeyJustPressed('G'))
    {
        m_enableChunkDebug = !m_enableChunkDebug;
        m_world->SetEnableChunkDebug(m_enableChunkDebug);
    }

    // F3 Enable Prolific GUI
    if (g_theInput->WasKeyJustPressed(0x72))
    {
        auto profiler = g_theGUI->GetGUI(std::type_index(typeid(GUIProfiler)));
        if (profiler)
            g_theGUI->RemoveFromViewPort(profiler);
        else g_theGUI->AddToViewPort(std::make_unique<GUIProfiler>());
    }

    // F3 + L Enable Light Debug GUI
    if (/*g_theInput->WasKeyJustPressed(0x72) &&*/ g_theInput->WasKeyJustPressed('L'))
    {
        auto lightDebugger = g_theGUI->GetGUI(std::type_index(typeid(GUIDebugLight)));
        if (lightDebugger)
            g_theGUI->RemoveFromViewPort(lightDebugger);
        else g_theGUI->AddToViewPort(std::make_unique<GUIDebugLight>());
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (m_isGameStart)
        {
            m_isInMainMenu = true;
            m_isGameStart  = false;
            g_theInput->SetCursorMode(CursorMode::POINTER);
            g_theEventSystem->FireEvent("Event.PlayerQuitWorld");
        }
        else
        {
            g_theEventSystem->FireEvent("WindowCloseEvent");
        }
    }

    if (g_theInput->WasKeyJustPressed('H') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
    {
        if (m_isGameStart)
        {
            m_player->m_position    = Vec3(-2, 0, 1);
            m_player->m_orientation = EulerAngles();
        }
    }

    if (g_theInput->WasKeyJustPressed('H'))
    {
        g_theSchedule->AddTask(new DummyTask("DummyTask", 5000));
    }

    if (m_isGameStart)
    {
        // 7
        if (g_theInput->WasKeyJustPressed(0x37))
        {
            DebugAddMessage(Stringf("Camera orientation: %.2f, %.2f, %.2f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), 5);
        }
    }
}

void Game::HandleMouseEvent(float deltaTime)
{
    UNUSED(deltaTime)
}


void Game::StartGame()
{
    printf("[core]         Game stated");
    m_isInMainMenu = false;
    m_isGameStart  = true;
    g_theInput->SetCursorMode(CursorMode::FPS);
    g_theEventSystem->FireEvent("Event.PlayerJoinWorld");
}


void Game::RegisterBlocks()
{
    using namespace enigma::registry::block;

    LogInfo(LogGame, "Starting block registration phase...");

    // Load blocks from the simpleminer namespace
    std::filesystem::path dataPath      = ".enigma\\data";
    std::string           namespaceName = "simpleminer";

    // Load all blocks from the simpleminer namespace
    BlockRegistry::LoadNamespaceBlocks(dataPath.string(), namespaceName);
    AIR = BlockRegistry::GetBlock("simpleminer", "air");
    LogInfo(LogGame, "Block registration completed!");
}
