#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Game/Framework/GUISubsystem.hpp"

struct Vertex_PCU;
class Player;

class GUIBlock3DSelection : public GUI
{
    DECLARE_GUI(GUIBlock3DSelection, "GUIBlock3DSelection", 80)

public:
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    GUIBlock3DSelection();
    ~GUIBlock3DSelection() override;

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    Player*                 m_player = nullptr;
    std::vector<Vertex_PCU> m_vertices;
    Vec3                    m_blockPosition;
    AABB3                   m_unitBlock = AABB3(Vec3(0, 0, 0), Vec3(1, 1, 1));
};
