#pragma once
#include "Engine/Voxel/Block/BlockState.hpp"
#include "Game/Framework/GUISubsystem.hpp"

class Timer;
class Player;

class GUIDebugLight : public GUI
{
public:
    DECLARE_GUI(GUIDebugLight, "GUIDebugLight", 90)

    GUIDebugLight();
    ~GUIDebugLight() override;

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    void PopulateDebugBlocks();

private:
    // [NEW] 存储方块状态和位置的结构体
    struct DebugBlockInfo
    {
        enigma::voxel::BlockState* state;
        enigma::voxel::BlockPos    pos;
    };

    std::vector<DebugBlockInfo> m_blocks;
    enigma::voxel::World*       m_world  = nullptr;
    Player*                     m_player = nullptr;
    Timer*                      m_timer  = nullptr;

public:
    int m_debugRadius = 8;
};
