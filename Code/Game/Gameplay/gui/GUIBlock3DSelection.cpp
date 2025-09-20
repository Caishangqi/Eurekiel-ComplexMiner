#include "GUIBlock3DSelection.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/entity/Player.hpp"

bool GUIBlock3DSelection::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)
    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUIBlock3DSelection)));
    if (gui) g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUIBlock3DSelection::GUIBlock3DSelection()
{
    m_vertices.reserve(1024);
}

GUIBlock3DSelection::~GUIBlock3DSelection()
{
}

void GUIBlock3DSelection::Draw()
{
}

void GUIBlock3DSelection::DrawHud()
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(Vec3((float)m_blockPosition.x, (float)m_blockPosition.y, (float)m_blockPosition.z));
    matTranslation.Append(Mat44::MakeNonUniformScale3D(Vec3::ONE));

    g_theRenderer->SetDepthMode(DepthMode::DISABLED);
    g_theRenderer->SetModelConstants(matTranslation, Rgba8::WHITE);
    g_theRenderer->SetBlendMode(BlendMode::ALPHA);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(m_vertices);
}

void GUIBlock3DSelection::Update(float deltaTime)
{
    UNUSED(deltaTime)
    m_hudCamera->SetPosition(m_player->m_position);
    m_hudCamera->SetOrientation(m_player->m_orientation);
}

void GUIBlock3DSelection::OnCreate()
{
    m_player = g_theGame->m_player;
    AddVertsForCube3DWireFrame(m_vertices, m_unitBlock, Rgba8::WHITE);
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
}

void GUIBlock3DSelection::OnDestroy()
{
}
