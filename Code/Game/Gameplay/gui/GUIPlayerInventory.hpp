#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Game/Framework/GUISubsystem.hpp"

namespace enigma::registry::block
{
    class Block;
}

struct Vertex_PCU;

class GUIPlayerInventory : public GUI
{
    DECLARE_GUI(GUIPlayerInventory, "GUIPlayerInventory", 80)

public:
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    explicit GUIPlayerInventory();
    ~GUIPlayerInventory() override;

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    std::shared_ptr<enigma::registry::block::Block> GetPreBlock();
    std::shared_ptr<enigma::registry::block::Block> GetCurrentBlock();
    std::shared_ptr<enigma::registry::block::Block> GetNextBlock();

    void ProcessInput(float deltaTime);

private:
    std::vector<Vertex_PCU>                                      m_vertices;
    std::vector<std::shared_ptr<enigma::registry::block::Block>> m_blocks;
    unsigned int                                                 m_currentIndex = 0;
    unsigned int                                                 m_numBlocks    = 0;
};
