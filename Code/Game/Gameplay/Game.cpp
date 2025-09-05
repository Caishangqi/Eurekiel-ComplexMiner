#include <Game/Gameplay/Game.hpp>

#include "../Framework/App.hpp"
#include "../GameCommon.hpp"
#include "../Player.hpp"
#include "../Prop.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"

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
#include "Engine/Audio/AudioSubsystem.hpp"
#include "Engine/Core/Yaml.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"


Game::Game()
{
    /// Resource
    g_theRenderer->CreateOrGetTexture(".enigma/assets/default/textures/test/TestUV.png");
    g_theRenderer->CreateOrGetTexture(".enigma/assets/default/textures/test/Caizii.png");

    /// Rasterize
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

    /// Spaces
    m_screenSpace.m_mins = Vec2::ZERO;
    m_screenSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.f));

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
    m_player             = new Player(this);
    m_player->m_position = Vec3(-2, 0, 1);
    /// 

    /// Cube
    m_cube   = new Prop(this);
    m_cube_1 = new Prop(this);

    AddVertsForCube3D(m_cube->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));
    AddVertsForCube3D(m_cube_1->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));

    m_cube->m_position   = Vec3(2, 2, 0);
    m_cube_1->m_position = Vec3(-2, -2, 0);
    ///

    /// Test Prop
    m_testProp             = new Prop(this);
    m_testProp->m_position = Vec3(0, 0, 0);
    //AddVertsForCylinder3D(m_testProp->m_vertexes,Vec3(0,2,0),Vec3(0,0,0),1);
    //AddVertsForCone3D(m_testProp->m_vertexes,Vec3(0,2,0),Vec3(0,0,0),1);
    AddVertsForArrow3D(m_testProp->m_vertexes, Vec3(0, 2, 0), Vec3(0, 0, 0), 0.1f, 0.4f);
    /// 

    /// Ball
    m_ball             = new Prop(this);
    m_ball->m_position = Vec3(10, -5, 1);
    m_ball->m_texture  = g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
    //m_ball->m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Caizii.png");
    AddVertsForSphere3D(m_ball->m_vertexes, Vec3(0, 0, 0), 2, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 64, 32);
    /// 

    /// Grid
    m_grid_x = new Prop(this);
    AddVertsForCube3D(m_grid_x->m_vertexes, Rgba8::RED);
    m_grid_x->m_scale = Vec3(GRID_SIZE * 2.f, 0.1f, 0.1f);
    m_grid_y          = new Prop(this);

    AddVertsForCube3D(m_grid_y->m_vertexes, Rgba8::GREEN);
    m_grid_y->m_scale = Vec3(0.1f, GRID_SIZE * 2.f, 0.1f);

    m_grid_x_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_x_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_5[i]->m_vertexes, Rgba8(191, 0, 0));
        m_grid_x_unit_5[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.06f, 0.06f);
        m_grid_x_unit_5[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0);
    }

    m_grid_x_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_x_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_x_unit_1[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.03f, 0.03f);
        m_grid_x_unit_1[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i), 0);
    }

    m_grid_y_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_y_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_5[i]->m_vertexes, Rgba8(0, 191, 0));
        m_grid_y_unit_5[i]->m_scale    = Vec3(0.06f, GRID_SIZE * 2.f, 0.06f);
        m_grid_y_unit_5[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0, 0);
    }

    m_grid_y_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_y_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_y_unit_1[i]->m_scale    = Vec3(0.03f, GRID_SIZE * 2.f, 0.03f);
        m_grid_y_unit_1[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i), 0, 0);
    }
    ///

    /// Debug Drawing

    // Arrows
    DebugAddWorldArrow(Vec3(1, 0, 0), Vec3(0, 0, 0), 0.12f, -1, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 1, 0), Vec3(0, 0, 0), 0.12f, -1, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 0, 1), Vec3(0, 0, 0), 0.12f, -1, Rgba8::BLUE, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    /// 

    // Text for y axis
    Mat44 transformY = Mat44::MakeTranslation3D(Vec3(0, 1.25f, 0.25f));
    transformY.AppendZRotation(180.f);
    DebugAddWorldText("y - left", transformY, 1.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for x axis
    Mat44 transformX = Mat44::MakeTranslation3D(Vec3(1.6f, 0, 0.25f));
    transformX.AppendZRotation(90.f);
    DebugAddWorldText("x - forward", transformX, 1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for z axis
    Mat44 transformZ = Mat44::MakeTranslation3D(Vec3(0, -0.25f, .9f));
    transformZ.AppendXRotation(-90.f);
    transformZ.AppendZRotation(180.f);
    DebugAddWorldText("z - up", transformZ, 1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);

    /// Game State
    g_theInput->SetCursorMode(CursorMode::POINTER);

    using enigma::core::YamlConfiguration;
    YamlConfiguration config = YamlConfiguration::LoadFromFile(".enigma/config/engine/module.yml");
    std::string       test   = config.GetString("moduleConfig.logger.globalLogLevel");

    /// Block Registration Phase - Moved to separate initialization method
    // InitializeBlocks(); // This will be called explicitly from App after subsystems are ready

    /// World Creation
    using namespace enigma::voxel::chunk;
    using namespace enigma::voxel::world;
    m_world = std::make_unique<World>("world", 0, std::make_unique<ChunkManager>());
    m_world->LoadChunk(0, 0);
    m_world->LoadChunk(2, 0);
    m_world->LoadChunk(2, 1);
    m_world->LoadChunk(2, -1);
}

Game::~Game()
{
    POINTER_SAFE_DELETE(m_grid_x)
    POINTER_SAFE_DELETE(m_grid_y)
    for (Prop* grid_x_unit_5 : m_grid_x_unit_5)
    {
        POINTER_SAFE_DELETE(grid_x_unit_5)
    }
    for (Prop* grid_x_unit_1 : m_grid_x_unit_1)
    {
        POINTER_SAFE_DELETE(grid_x_unit_1)
    }
    for (Prop* grid_y_unit_5 : m_grid_y_unit_5)
    {
        POINTER_SAFE_DELETE(grid_y_unit_5)
    }
    for (Prop* grid_y_unit_1 : m_grid_y_unit_1)
    {
        POINTER_SAFE_DELETE(grid_y_unit_1)
    }
    POINTER_SAFE_DELETE(m_ball)
    POINTER_SAFE_DELETE(m_cube_1)
    POINTER_SAFE_DELETE(m_cube)
    POINTER_SAFE_DELETE(m_player)
    POINTER_SAFE_DELETE(m_screenCamera)
    POINTER_SAFE_DELETE(m_worldCamera)
    POINTER_SAFE_DELETE(m_worldCamera)
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
        /// Grid
        RenderGrids();
        /// Props
        RenderProps();
        DebugRenderWorld(*g_theGame->m_player->m_camera);
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
        DebugDrawRing(Vec2(800, 400), m_currentIconCircleThickness, m_currentIconCircleThickness / 10, Rgba8::WHITE);
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

void Game::RenderGrids() const
{
    m_grid_x->Render();
    m_grid_y->Render();
    for (Prop* grid_x_unit : m_grid_x_unit_5)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_x_unit : m_grid_x_unit_1)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_5)
    {
        grid_y_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_1)
    {
        grid_y_unit->Render();
    }
}

void Game::RenderProps() const
{
    g_theRenderer->BindTexture(nullptr);
    m_cube->Render();
    m_cube_1->Render();
    m_ball->Render();
    //m_testProp->Render();
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
    m_cube_1->m_color = color;
    m_cube->m_orientation.m_rollDegrees += m_clock->GetDeltaSeconds() * 30;
    m_cube->m_orientation.m_pitchDegrees += m_clock->GetDeltaSeconds() * 30;
    /// 

    /// Sphere
    m_ball->m_orientation.m_yawDegrees += m_clock->GetDeltaSeconds() * 45;
    /// 

    /// Debug Only
    std::string debugGameState = Stringf("Time: %.2f FPS: %.1f Scale: %.2f",
                                         m_clock->GetTotalSeconds(),
                                         m_clock->GetFrameRate(),
                                         m_clock->GetTimeScale()
    );
    DebugAddScreenText(debugGameState, m_screenSpace, 14, 0);
    DebugAddMessage(Stringf("Player position: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z), 0);

    /// Display Only
#ifdef COSMIC
    m_counter++;
    m_currentIconCircleThickness = FluctuateValue(m_iconCircleRadius, 50.f, 0.02f, static_cast<float>(m_counter));
#endif
    float deltaTime = m_clock->GetDeltaSeconds();
    UpdateCameras(deltaTime);

    if (m_isGameStart)
    {
        HandleEntityCollisions();
    }

    HandleMouseEvent(deltaTime);
    HandleKeyBoardEvent(deltaTime);

    GarbageCollection();
}

void Game::UpdateWorld()
{
    if (m_world)
        m_world->Update(m_clock->GetDeltaSeconds());
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
    if (g_theInput->WasKeyJustPressed(0x72))
    {
        m_enableChunkDebug = !m_enableChunkDebug;
        m_world->SetEnableChunkDebug(m_enableChunkDebug);
    }

    if (g_theInput->WasKeyJustPressed('K'))
    {
        using namespace enigma::resource;
        g_theAudio->PlaySound(ResourceLocation::Of("engine", "sounds/mono/laser"));
        //g_theAudio->PlaySound(ResourceLocation::Parse("engine:sounds/mono/laser"));
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (m_isGameStart)
        {
            m_isInMainMenu = true;
            m_isGameStart  = false;
            g_theInput->SetCursorMode(CursorMode::POINTER);
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

    if (m_isGameStart)
    {
        // 1
        if (g_theInput->WasKeyJustPressed(0x31))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            DebugAddWorldCylinder(m_player->m_position, m_player->m_position + forward * 20, 0.0625f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
        }
        if (g_theInput->IsKeyDown(0x32))
        {
            DebugAddWorldSphere(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), 0.25f, 10.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0));
        }
        // 3
        if (g_theInput->WasKeyJustPressed(0x33))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            // Push the wire ball 2 unit forward away
            DebugAddWorldWireSphere(m_player->m_position + forward * 2, 1, 5.f, Rgba8::GREEN, Rgba8::RED);
        }
        // 4
        if (g_theInput->WasKeyJustPressed(0x34))
        {
            Mat44 transform = Mat44::MakeTranslation3D(m_player->m_position);
            transform.Append(m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            DebugAddWorldBasis(transform, 20.f);
        }

        // 5
        if (g_theInput->WasKeyJustPressed(0x35))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            DebugAddWorldBillboardText(
                Stringf("Position: %.2f, %.2f, %.2f Orientation: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, m_player->m_orientation.m_yawDegrees,
                        m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), m_player->m_position + forward * 2, 0.125f, Rgba8::WHITE, Rgba8::RED,
                DebugRenderMode::USE_DEPTH,
                Vec2(0.5, 0.5), 10.f);
        }

        // 6
        if (g_theInput->WasKeyJustPressed(0x36))
        {
            DebugAddWorldCylinder(m_player->m_position + Vec3(0, 0, 1), m_player->m_position, 0.5f, 10, Rgba8::WHITE, Rgba8::RED);
        }

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
}


void Game::RenderEntities() const
{
}

void Game::HandleEntityCollisions()
{
}

void Game::GarbageCollection()
{
}

void Game::InitializeBlocks()
{
    using namespace enigma::registry::block;

    LogInfo("game", "Starting block registration phase...");

    // Load blocks from the simpleminer namespace
    std::filesystem::path dataPath      = ".enigma\\data";
    std::string           namespaceName = "simpleminer";

    //LogInfo("game", "Loading blocks from: %s / %s / block /", dataPath.string().c_str(), namespaceName.c_str());

    // Load all blocks from the simpleminer namespace
    BlockRegistry::LoadNamespaceBlocks(dataPath.string(), namespaceName);

    // Print registered blocks for verification
    //auto blockNames = BlockRegistry::GetBlockNames(namespaceName);
    //LogInfo("game", "Registered %d blocks from namespace '%s':", blockNames.size(), namespaceName.c_str());

    // Also check all blocks (without namespace filter)
    /*
    auto allBlockNames = BlockRegistry::GetBlockNames();
    LogInfo("game", "Total registered blocks: %d", allBlockNames.size());
    for (const auto& blockName : allBlockNames)
    {
        LogInfo("game", "  - Found block: %s", blockName.c_str());
    }
    */

    /*for (const auto& blockName : blockNames)
    {
        auto block = BlockRegistry::GetBlock(namespaceName, blockName);
        if (block)
        {
            LogInfo("game", "  - %s:%s (properties: %d, states: %d)", namespaceName.c_str(), blockName.c_str(), block->GetProperties().size(), block->GetStateCount());
        }
    }*/
    LogInfo("game", "Block registration completed!");
}
