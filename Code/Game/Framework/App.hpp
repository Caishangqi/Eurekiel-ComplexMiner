#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"

class Window;
class Game;

// Forward declaration for resource system
namespace enigma::resource
{
    class ResourceSubsystem;
}

extern Window* g_theWindow;

class App
{
public:
    App();
    ~App();

    void Startup(char* commandLineString = nullptr);
    void Shutdown();
    void RunFrame();

    bool IsQuitting() const;
    void HandleQuitRequested();

    void AdjustForPauseAndTimeDistortion();
    void HandleKeyBoardEvent();

    /// Event Handle
    static bool Event_ConsoleStartup(EventArgs& args);
    /// 
private:
    void BeginFrame();
    void UpdateCameras();
    void Update();

    void Render() const;
    void EndFrame();

    /// TinyXML2 to parse the (newly-created) file Data/GameConfig.xml (if it exists), and populate the game config
    /// blackboard from that file’s root XML element’s attributes. Each game project should have its own GameConfig.xml
    /// file;
    void LoadGameConfig(const char* filename);

public:
    bool  m_isQuitting       = false;
    bool  m_isPaused         = false;
    bool  m_isSlowMo         = false;
    bool  m_isDebug          = false;
    bool  m_isPendingRestart = false;
    Rgba8 m_backgroundColor  = Rgba8(63, 63, 63, 255);

    AABB2 m_consoleSpace;

    STATIC bool WindowCloseEvent(EventArgs& args);
};
