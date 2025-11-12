#include "GUICrosser.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/entity/Player.hpp"

Crosser::Crosser()
{
    AddVertsForArrow3D(m_vertexes, Vec3(0.1f, 0, 0), Vec3(), 0.004f, 0.4f, Rgba8::RED);
    AddVertsForArrow3D(m_vertexes, Vec3(0, 0.1f, 0), Vec3(), 0.004f, 0.4f, Rgba8::GREEN);
    AddVertsForArrow3D(m_vertexes, Vec3(0, 0, 0.1f), Vec3(), 0.004f, 0.4f, Rgba8::BLUE);
}

void Crosser::Update(float deltaTime)
{
    UNUSED(deltaTime)
}

Mat44 Crosser::GetModelToWorldTransform()
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    matTranslation.Append(Mat44::MakeNonUniformScale3D(m_scale));
    return matTranslation;
}

void Crosser::Renderer()
{
    g_theRenderer->SetDepthMode(depth_mode::DISABLED);
    g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_color);
    g_theRenderer->SetBlendMode(blend_mode::ALPHA);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(m_vertexes);
}

bool GUICrosser::Event_GameInstanceRemove(EventArgs& args)
{
    UNUSED(args)
    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUICrosser)));
    g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUICrosser::GUICrosser(Player* player) : GUI(), m_player(player)
{
    m_crosser             = std::make_unique<Crosser>();
    m_crosser->m_position = m_player->m_position;
}

void GUICrosser::Draw()
{
}

void GUICrosser::DrawHud()
{
    m_crosser->Renderer();
}

void GUICrosser::Update(float deltaTime)
{
    m_crosser->Update(deltaTime);
    Vec3 iFwd, jLeft, kUp;
    m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(iFwd, jLeft, kUp);
    float offsetDistance  = 2.f;
    Vec3  debugAxisPos    = m_player->m_position + iFwd * offsetDistance;
    m_crosser->m_position = debugAxisPos;

    m_hudCamera->SetPosition(m_player->m_position);
    m_hudCamera->SetOrientation(m_player->m_orientation);
}

void GUICrosser::OnCreate()
{
    g_theEventSystem->SubscribeEventCallbackFunction("Event.Game.GameInstanceRemove", Event_GameInstanceRemove);
}

void GUICrosser::OnDestroy()
{
}
