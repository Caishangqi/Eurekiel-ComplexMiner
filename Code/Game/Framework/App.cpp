#include "Game/Framework/App.hpp"

#include "Engine/Window/Window.hpp"
#include "../Gameplay/Game.hpp"
#include "Engine/Audio/AudioSubsystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Resource system integration
#include "Engine/Resource/ResourceSubsystem.hpp"
#include "Engine/Resource/ResourceCommon.hpp"

// Model system integration  
#include "Engine/Model/ModelSubsystem.hpp"

// Logger system integration
#include "Engine/Core/Logger/Logger.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"

// Console system integration
#include "GUISubsystem.hpp"
#include "Engine/Core/Console/ConsoleSubsystem.hpp"
#include "Engine/Registry/Core/RegisterSubsystem.hpp"

// Window configuration parser
#include "WindowConfigParser.hpp"

// Windows API for testing
#ifdef _WIN32
#include <windows.h>
#endif

Window*                              g_theWindow   = nullptr;
IRenderer*                           g_theRenderer = nullptr;
App*                                 g_theApp      = nullptr;
RandomNumberGenerator*               g_rng         = nullptr;
InputSystem*                         g_theInput    = nullptr;
AudioSubsystem*                      g_theAudio    = nullptr;
Game*                                g_theGame     = nullptr;
enigma::resource::ResourceSubsystem* g_theResource = nullptr;
GUISubsystem*                        g_theGUI      = nullptr;

App::App()
{
    // Create Engine instance
    enigma::core::Engine::CreateInstance();
}

App::~App()
{
}

void App::Startup(char*)
{
    using namespace enigma::resource;

    // Load Game Config
    LoadConfigurations();

    EventSystemConfig eventSystemConfig;
    g_theEventSystem = new EventSystem(eventSystemConfig);
    g_theEventSystem->SubscribeEventCallbackFunction("WindowCloseEvent", WindowCloseEvent); // Subscribe the WindowCloseEvent
    g_theEventSystem->SubscribeEventCallbackFunction("Event.Console.Startup", Event_ConsoleStartup);

    // Create All Engine Subsystems
    InputSystemConfig inputConfig;
    g_theInput = new InputSystem(inputConfig);

    // Load window configuration from settings.yml
    WindowConfig windowConfig  = WindowConfigParser::LoadFromYaml(".enigma/settings.yml");
    windowConfig.m_inputSystem = g_theInput;
    g_theWindow                = new Window(windowConfig);

    RenderConfig renderConfig;
    renderConfig.m_window        = g_theWindow;
    renderConfig.m_defaultShader = ".enigma\\data\\Shaders\\Default2D";
    renderConfig.m_backend       = RendererBackend::DirectX11;
    g_theRenderer                = IRenderer::CreateRenderer(renderConfig); // Create render

    DebugRenderConfig debugRenderConfig;
    debugRenderConfig.m_renderer = g_theRenderer;

    m_consoleSpace.m_mins   = Vec2::ZERO;
    m_consoleSpace.m_maxs.x = (float)windowConfig.m_resolution.x;
    m_consoleSpace.m_maxs.y = (float)windowConfig.m_resolution.y;

    DevConsoleConfig consoleConfig;
    consoleConfig.renderer         = g_theRenderer;
    consoleConfig.m_camera         = new Camera();
    consoleConfig.m_camera->m_mode = eMode_Orthographic;
    consoleConfig.m_camera->SetOrthographicView(m_consoleSpace.m_mins, m_consoleSpace.m_maxs); // TODO: Consider use Client Dimension fetch from Windows, after windows startup
    g_theDevConsole = new DevConsole(consoleConfig);

    // Register Engine subsystems
    using namespace enigma::core;
    using namespace enigma::resource;

    // Create and register RegisterSubsystem first (needed for block registry)
    auto registerSubsystem = std::make_unique<RegisterSubsystem>();
    GEngine->RegisterSubsystem(std::move(registerSubsystem));

    // Create and register LoggerSubsystem first (highest priority)
    auto logger = std::make_unique<LoggerSubsystem>();
    GEngine->RegisterSubsystem(std::move(logger));

    // Create and register ConsoleSubsystem (high priority, after logger)
    auto consoleSubsystem = std::make_unique<ConsoleSubsystem>();
    GEngine->RegisterSubsystem(std::move(consoleSubsystem));

    // Create ResourceSubsystem with configuration
    ResourceConfig resourceConfig;
    resourceConfig.baseAssetPath    = ".enigma/assets";
    resourceConfig.enableHotReload  = true;
    resourceConfig.logResourceLoads = true;
    resourceConfig.printScanResults = true;
    resourceConfig.AddNamespace("simpleminer", ""); // Add custom namespaces

    auto resourceSubsystem = std::make_unique<ResourceSubsystem>(resourceConfig);
    GEngine->RegisterSubsystem(std::move(resourceSubsystem));

    // Create ModelSubsystem (depends on ResourceSubsystem and RenderSubsystem)
    auto modelSubsystem = std::make_unique<enigma::model::ModelSubsystem>();
    GEngine->RegisterSubsystem(std::move(modelSubsystem));

    // Create AudioSubsystem with configuration
    AudioSystemConfig audioConfig;
    audioConfig.enableResourceIntegration = true;
    audioConfig.resourceSubsystem         = resourceSubsystem.get();
    auto audioSubsystem                   = std::make_unique<AudioSubsystem>(audioConfig);
    GEngine->RegisterSubsystem(std::move(audioSubsystem));

    // Create GUISubsystem with configuration
    GUIConfig guiConfig;
    guiConfig.screenSpace = g_theApp->m_consoleSpace;
    auto guiSubsystem     = std::make_unique<GUISubsystem>(guiConfig);
    GEngine->RegisterSubsystem(std::move(guiSubsystem));

    // Set up global pointers for legacy compatibility
    g_theResource = GEngine->GetSubsystem<ResourceSubsystem>();
    g_theAudio    = GEngine->GetSubsystem<AudioSubsystem>();
    g_theGUI      = GEngine->GetSubsystem<GUISubsystem>();

    g_theEventSystem->Startup();

    // Start Engine subsystems first (includes ConsoleSubsystem)

    g_theWindow->Startup();
    g_theRenderer->Startup();
    g_theDevConsole->Startup();
    g_theInput->Startup();

    GEngine->Startup();
    DebugRenderSystemStartup(debugRenderConfig);

    g_theGame = new Game();

    g_rng = new RandomNumberGenerator();
}

void App::Shutdown()
{
    /*
     *  All Destroy and ShutDown process should be reverse order of the StartUp
     */

    // Destroy the game
    delete g_theGame;
    g_theGame = nullptr;

    // Shutdown Engine subsystems (handles ResourceSubsystem and AudioSubsystem)
    GEngine->Shutdown();

    // Clear global pointers (Engine manages the objects now)
    g_theResource = nullptr;
    g_theAudio    = nullptr;

    g_theDevConsole->Shutdown();
    DebugRenderSystemShutdown();
    g_theRenderer->Shutdown();
    g_theWindow->Shutdown();
    g_theInput->Shutdown();
    g_theEventSystem->Shutdown();

    // Destroy remaining Engine Subsystems (not managed by Engine yet)
    delete g_theDevConsole;
    g_theDevConsole = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;

    delete g_theWindow;
    g_theWindow = nullptr;

    delete g_theInput;
    g_theInput = nullptr;

    delete g_theEventSystem;
    g_theEventSystem = nullptr;

    // Destroy Engine instance
    enigma::core::Engine::DestroyInstance();
}

void App::RunFrame()
{
    BeginFrame(); //Engine pre-frame stuff
    Update(); // Game updates / moves / spawns / hurts
    Render(); // Game draws current state of things
    EndFrame(); // Engine post-frame
}

bool App::IsQuitting() const
{
    return m_isQuitting;
}

void App::HandleQuitRequested()
{
    m_isQuitting = true;
}

void App::HandleKeyBoardEvent()
{
    // ESC compatibility controller
    if (g_theInput->GetController(0).WasButtonJustPressed(XBOX_BUTTON_B))
    {
        if (g_theGame->m_isInMainMenu)
        {
            m_isQuitting = true;
        }
    }

    // ESC
    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
    {
        if (g_theGame->m_isInMainMenu)
        {
            m_isQuitting = true;
        }
    }

    if (g_theInput->WasKeyJustPressed(0x70))
    {
        m_isDebug = !m_isDebug;
    }

    // Reload the Game, Delete and Create
    if (g_theInput->WasKeyJustPressed(0x77))
    {
        m_isPendingRestart = true;
    }
    if (g_theInput->WasKeyJustPressed('O'))
    {
        m_isPaused = true;
        g_theGame->m_clock->StepSingleFrame();
    }
}

void App::LoadConfigurations()
{
    settings = YamlConfiguration::LoadFromFile(".enigma/settings.yml");
}

bool App::Event_ConsoleStartup(EventArgs& args)
{
    UNUSED(args)
    using namespace enigma::core;
    LogInfo("Game", "This is an example log info test.");

    // Output to DevConsole (will be mirrored to IDE Console via DevConsoleAppender)
    g_theDevConsole->AddLine(Rgba8(95, 95, 95),
                             "Mouse        - Aim\n"
                             "W/A          - Move\n"
                             "S/D          - Strafe\n"
                             "Q/E          - Down | Up\n"
                             "Shift        - Sprint\n"
                             "LMB          - Place select block\n"
                             "RMB          - Break block under player\n"
                             "Wheel Up     - Select Previous block\n"
                             "Wheel Down   - Select Next block\n"
                             "F8           - Reload the Game\n"
                             "F3           - Toggle Chunk Pool Statistic\n"
                             "F3 + G       - Toggle Chunk Boarder\n"
                             "ESC          - Quit\n"
                             "P            - Pause the Game\n"
                             "O            - Step single frame\n"
                             "T            - Toggle time scale between 0.1 and 1.0\n"
                             "~            - Toggle Develop Console");
    return true;
}

void App::AdjustForPauseAndTimeDistortion()
{
    m_isSlowMo = g_theInput->IsKeyDown('T');
    if (m_isSlowMo)
    {
        g_theGame->m_clock->SetTimeScale(0.1f);
    }
    else
    {
        g_theGame->m_clock->SetTimeScale(1.0f);
    }


    if (g_theInput->WasKeyJustPressed('P'))
    {
        m_isPaused = !m_isPaused;
    }

    if (m_isPaused)
    {
        g_theGame->m_clock->Pause();
    }
    else
    {
        g_theGame->m_clock->Unpause();
    }
}


void App::BeginFrame()
{
    Clock::TickSystemClock();
    g_theInput->BeginFrame();
    g_theWindow->BeginFrame();
    g_theRenderer->BeginFrame();
    DebugRenderBeginFrame();
    g_theAudio->BeginFrame();
    g_theEventSystem->BeginFrame();
    g_theDevConsole->BeginFrame();
}

void App::UpdateCameras()
{
}

void App::Update()
{
    // Calculate delta time
    static float lastTime    = 0.0f;
    float        currentTime = Clock::GetSystemClock().GetTotalSeconds();
    float        deltaTime   = currentTime - lastTime;
    lastTime                 = currentTime;

    // Update Engine subsystems (including ConsoleSubsystem)
    GEngine->Update(deltaTime);

    // Update resource system for hot reload
    g_theResource->Update();

    /// Cursor
    auto windowHandle   = static_cast<HWND>(g_theWindow->GetWindowHandle());
    bool windowHasFocus = (GetActiveWindow() == windowHandle);

    bool devConsoleOpen = (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN);
    if (!windowHasFocus || devConsoleOpen)
    {
        g_theInput->SetCursorMode(CursorMode::POINTER);
    }
    else
    {
        g_theInput->SetCursorMode(CursorMode::FPS);
    }

    /// 

    HandleKeyBoardEvent();
    AdjustForPauseAndTimeDistortion();
    g_theGame->Update();
}

// If this methods is const, the methods inn the method should also promise
// const
void App::Render() const
{
    g_theRenderer->ClearScreen(Rgba8(m_backgroundColor));
    g_theGame->Render();
    g_theGUI->Render();
    g_theDevConsole->Render(m_consoleSpace);
}

void App::EndFrame()
{
    g_theWindow->EndFrame();
    g_theRenderer->EndFrame();
    DebugRenderEndFrame();
    g_theInput->EndFrame();
    g_theAudio->EndFrame();
    g_theEventSystem->EndFrame();
    g_theDevConsole->EndFrame();
    g_theGUI->EndFrame();

    if (m_isPendingRestart)
    {
        delete g_theGame;
        g_theGame = nullptr;
        g_theGame = new Game();
        // Restore state
        m_isPendingRestart = false;
        m_isPaused         = false;
    }
}

bool App::WindowCloseEvent(EventArgs& args)
{
    UNUSED(args)
    g_theApp->HandleQuitRequested();
    return false;
}
