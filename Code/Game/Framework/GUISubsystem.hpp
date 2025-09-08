#pragma once
#include "Engine/Core/SubsystemManager.hpp"
#include "Engine/Math/AABB2.hpp"

class BitmapFont;
class Camera;
using namespace enigma::core;

struct GUIConfig
{
    AABB2 screenSpace = AABB2::ZERO_TO_ONE;
};


class GUI
{
    friend class GUISubsystem;

public:
    virtual             ~GUI() = default;
    virtual const char* GetName() const = 0;
    virtual int         GetPriority() const = 0;
    virtual void        Draw() = 0;
    virtual void        DrawHud() = 0;
    virtual void        Update(float deltaTime) = 0;
    virtual void        OnCreate() = 0;
    virtual void        OnDestroy() = 0;

#define DECLARE_GUI(ClassName, Name, Priority) \
    public:\
            const char* GetName() const override { return Name; } \
            int GetPriority() const override { return Priority; }

protected:
    std::shared_ptr<Camera> m_guiCamera      = nullptr;
    std::shared_ptr<Camera> m_hudCamera      = nullptr;
    BitmapFont*             m_defaultGUIFont = nullptr;
    GUIConfig               m_config;

private:
    bool m_hasCreate      = false;
    bool m_pendingDestroy = false;
};


class GUISubsystem : public EngineSubsystem
{
public:
    DECLARE_SUBSYSTEM(GUISubsystem, "guiSubsystem", 400)

    explicit GUISubsystem(GUIConfig& config);

    void Render();
    void Update(float deltaTime) override;
    void EndFrame() override;
    void Startup() override;
    void Shutdown() override;

private:
    void RenderHud();
    void RenderGUI();

public:
    [[maybe_unused]] std::shared_ptr<GUI> AddToViewPort(std::shared_ptr<GUI> gui);
    [[maybe_unused]] std::shared_ptr<GUI> RemoveFromViewPort(std::shared_ptr<GUI> gui);
    std::shared_ptr<GUI>                  GetGUI(const std::type_index& type);
    std::shared_ptr<GUI>                  GetGUI(std::string& name);

private:
    GUIConfig                         m_config;
    std::vector<std::shared_ptr<GUI>> m_guis;
    std::shared_ptr<Camera>           m_guiCamera      = nullptr;
    std::shared_ptr<Camera>           m_hudCamera      = nullptr;
    BitmapFont*                       m_defaultGUIFont = nullptr;
};
