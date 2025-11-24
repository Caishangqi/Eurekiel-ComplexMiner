#include "GUIPlayerStats.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/Player/Player.hpp"
#include "Game/Gameplay/Player/GameCamera.hpp"
#include "Game/Gameplay/Player/CameraMode.hpp"
#include "Game/Framework/Entity/PhysicsMode.hpp"

bool GUIPlayerStats::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)

    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUIPlayerStats)));
    if (gui) g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUIPlayerStats::GUIPlayerStats() : GUI()
{
    // Event
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
    m_vertices.reserve(1024);
}

void GUIPlayerStats::Draw()
{
    g_theRenderer->BindTexture(m_fontTexture);
    g_theRenderer->DrawVertexArray(m_vertices);
}

void GUIPlayerStats::DrawHud()
{
}

void GUIPlayerStats::Update(float deltaTime)
{
    UNUSED(deltaTime)

    // Get player reference
    if (!m_player)
    {
        m_player = g_theGame->m_player;
        if (!m_player) return; // Player not created yet
    }

    m_vertices.clear();

    // Get screen dimensions
    float screenWidth  = m_config.screenSpace.m_maxs.x;
    float screenHeight = m_config.screenSpace.m_maxs.y;

    // Left top corner information (from top to bottom)

    // Camera Mode
    std::string cameraModeText = "Camera: ";
    cameraModeText             += GetCameraModeName(m_player->GetCamera()->GetCameraMode());
    cameraModeText             += " [C]";
    AABB2 cameraModeBox(Vec2(0, screenHeight - 36), Vec2(screenWidth, screenHeight - 20));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, cameraModeText, cameraModeBox, 12.0f, Rgba8::WHITE, 1, Vec2(0.0f, 1.0f));

    // Physics Mode
    std::string physicsModeText = "Physics: ";
    physicsModeText             += GetPhysicsModeName(m_player->GetPhysicsMode());
    physicsModeText             += " [V]";
    AABB2 physicsModeBox        = cameraModeBox;
    physicsModeBox.SetCenter(cameraModeBox.GetCenter() + Vec2(0, -12));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, physicsModeText, physicsModeBox, 12.0f, Rgba8::WHITE, 1, Vec2(0.0f, 1.0f));
}

void GUIPlayerStats::OnCreate()
{
    m_fontTexture = &m_defaultGUIFont->GetTexture();
    m_player      = g_theGame->m_player;
}

void GUIPlayerStats::OnDestroy()
{
    m_player = nullptr;
}
