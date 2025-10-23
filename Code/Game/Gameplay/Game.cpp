#include <Game/Gameplay/Game.hpp>

#include "../Framework/App.hpp"
#include "../GameCommon.hpp"
#include "entity/Player.hpp"
#include "SimpleMinerGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"

#include "Engine/Math/Vec2.hpp"
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
#include "Engine/Voxel/Builtin/DefaultBlock.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Framework/DummyTask.hpp"
#include "Game/Framework/GUISubsystem.hpp"
#include "gui/GUIProfiler.hpp"


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
    m_clock = new Clock(Clock::GetSystemClock());
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
    m_world            = std::make_unique<World>("world", 0, std::move(generator));
    int renderDistance = settings.GetInt("video.simulationDistance", 32);
    m_world->SetChunkActivationRange(renderDistance);
    LogInfo(LogGame, "Render distance configured: %d chunks", renderDistance);
}

Game::~Game()
{
    // Save and close world before cleanup
    if (m_world)
    {
        LogInfo(LogGame, "Saving world before game shutdown...");
        m_world->SaveWorld();
        m_world->CloseWorld();
        m_world.reset(); // Explicitly release the world
    }

    POINTER_SAFE_DELETE(m_player)
    POINTER_SAFE_DELETE(m_screenCamera)
    POINTER_SAFE_DELETE(m_worldCamera)
    POINTER_SAFE_DELETE(m_worldCamera)

    g_theEventSystem->FireEvent("Event.Game.GameInstanceRemove");
}


void Game::Render() const
{
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetBlendMode(BlendMode::ALPHA);
    g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
    if (!m_isInMainMenu)
    {
        m_player->Render();
        RenderWorld();
        DebugRenderScreen(*g_theGame->m_screenCamera);
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
    if (m_world && m_world->GetChunkManager())
    {
        auto* chunkManager = m_world->GetChunkManager().get();
        UNUSED(chunkManager)
        // Calculate player's current chunk coordinates using proper constants
        int32_t playerChunkX = static_cast<int32_t>(std::floor(m_player->m_position.x / enigma::voxel::Chunk::CHUNK_SIZE_X));
        int32_t playerChunkY = static_cast<int32_t>(std::floor(m_player->m_position.y / enigma::voxel::Chunk::CHUNK_SIZE_Y));

        UNUSED(playerChunkX)
        UNUSED(playerChunkY)

        // Display chunk management stats
        //DebugAddMessage(Stringf("Chunk: (%d, %d) | Loaded: %zu chunks",playerChunkX, playerChunkY, chunkManager->GetLoadedChunkCount()), 1);

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
            m_world->GetChunkManager()->SetPlayerPosition(m_player->m_position);
        }

        m_world->Update(m_clock->GetDeltaSeconds());
    }
}

void Game::RenderWorld() const
{
    if (m_world)
        m_world->Render(g_theRenderer);
}


void Game::HandleKeyBoardEvent(float deltaTime)
{
    UNUSED(deltaTime)
    const XboxController& controller = g_theInput->GetController(0);


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
