#pragma once
#include "Game/Framework/GUISubsystem.hpp"
#include <vector>

#include "Engine/Core/EventSystem.hpp"

struct Vertex_PCU;
class Texture;
class Player;

class GUIPlayerStats : public GUI
{
    DECLARE_GUI(GUIPlayerStats, "GUIPlayerStats", 110)

public:
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    explicit GUIPlayerStats();

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    Texture*                m_fontTexture = nullptr;
    std::vector<Vertex_PCU> m_vertices;
    Player*                 m_player = nullptr;
};
