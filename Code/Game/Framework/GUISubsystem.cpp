#include "GUISubsystem.hpp"

#include "Engine/Core/Logger/LoggerAPI.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"

GUISubsystem::GUISubsystem(GUIConfig& config) : m_config(config)
{
}

void GUISubsystem::Render()
{
    RenderGUI();
    RenderHud();
}

void GUISubsystem::Update(float deltaTime)
{
    EngineSubsystem::Update(deltaTime);
    for (auto& gui : m_guis)
    {
        if (gui && !gui->m_pendingDestroy)
            gui->Update(deltaTime);
    }
}

void GUISubsystem::EndFrame()
{
    EngineSubsystem::EndFrame();
    for (auto& gui : m_guis)
    {
        if (gui && gui->m_pendingDestroy)
        {
            gui.reset();
            gui = nullptr;
        }
    }
}

void GUISubsystem::Startup()
{
    LogInfo("GUISubsystem", "Starting up GUISubsystem...");
    // 3D Camera, use for HUD
    m_hudCamera         = std::make_unique<Camera>();
    m_hudCamera->m_mode = eMode_Perspective;
    m_hudCamera->SetPerspectiveView(2.0f, 60.f, 0.1f, 100.f);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_hudCamera->SetCameraToRenderTransform(ndcMatrix);

    // 2D Camera, use for GUI
    m_guiCamera         = std::make_unique<Camera>();
    m_guiCamera->m_mode = eMode_Orthographic;
    m_guiCamera->SetOrthographicView(m_config.screenSpace.m_mins, m_config.screenSpace.m_maxs);

    // Font
    m_defaultGUIFont = g_theRenderer->CreateOrGetBitmapFont(".enigma\\data\\Fonts\\SquirrelFixedFont");
}

void GUISubsystem::Shutdown()
{
    LogInfo("GUISubsystem", "Shutdown GUISubsystem...");
}

void GUISubsystem::RenderHud()
{
    g_theRenderer->BeginCamera(*m_hudCamera);
    for (auto& gui : m_guis)
    {
        if (gui && !gui->m_pendingDestroy)
            gui->DrawHud();
    }
    g_theRenderer->EndCamera(*m_hudCamera);
}

void GUISubsystem::RenderGUI()
{
    g_theRenderer->BeginCamera(*m_guiCamera);
    for (auto& gui : m_guis)
    {
        if (gui && !gui->m_pendingDestroy)
            gui->Draw();
    }
    g_theRenderer->EndCamera(*m_guiCamera);
}

std::shared_ptr<GUI> GUISubsystem::AddToViewPort(std::shared_ptr<GUI> gui)
{
    if (gui && !gui->m_hasCreate && !gui->m_pendingDestroy)
    {
        m_guis.push_back(gui);
        gui->m_guiCamera      = m_guiCamera;
        gui->m_hudCamera      = m_hudCamera;
        gui->m_config         = m_config;
        gui->m_defaultGUIFont = m_defaultGUIFont;
        gui->OnCreate();
        LogInfo("gui", "Add %s GUI to ViewPort", gui->GetName());
        return gui;
    }
    else
    {
        LogError("gui", "Fail to add %s GUI to ViewPort");
    }
    return nullptr;
}

std::shared_ptr<GUI> GUISubsystem::RemoveFromViewPort(std::shared_ptr<GUI> gui)
{
    for (std::shared_ptr<GUI>& ptr : m_guis)
    {
        if (ptr == gui)
        {
            ptr->OnDestroy();
            ptr->m_pendingDestroy = true;
            LogInfo("gui", "Remove %s GUI from ViewPort", gui->GetName());
            return ptr;
        }
    }
    return nullptr;
}

std::shared_ptr<GUI> GUISubsystem::GetGUI(const std::type_index& type)
{
    for (std::shared_ptr<GUI>& gui : m_guis)
    {
        if (gui && std::type_index(typeid(*gui)) == type)
            return gui;
    }
    return nullptr;
}

std::shared_ptr<GUI> GUISubsystem::GetGUI(std::string& name)
{
    for (std::shared_ptr<GUI>& gui : m_guis)
    {
        if (gui->GetName() == name)
            return gui;
    }
    return nullptr;
}
