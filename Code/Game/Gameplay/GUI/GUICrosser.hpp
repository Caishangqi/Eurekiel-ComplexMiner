#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Framework/GUISubsystem.hpp"


struct Vertex_PCU;
class Player;

struct Crosser
{
    explicit Crosser();

    void  Update(float deltaTime);
    Mat44 GetModelToWorldTransform();
    void  Renderer();

    std::vector<Vertex_PCU> m_vertexes;
    Vec3                    m_position = Vec3::ZERO;
    EulerAngles             m_orientation;
    Vec3                    m_scale = Vec3::ONE;
    Rgba8                   m_color = Rgba8::WHITE;
};

class GUICrosser : public GUI
{
public:
    DECLARE_GUI(GUICrosser, "GUICrosser", 100)

    static bool Event_GameInstanceRemove(EventArgs& args);

public:
    explicit GUICrosser(Player* player);
    ~GUICrosser() override;

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    Player*                  m_player  = nullptr;
    std::unique_ptr<Crosser> m_crosser = nullptr;
};
