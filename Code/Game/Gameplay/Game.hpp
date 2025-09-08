#pragma once
#include "../GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"

namespace enigma::voxel::world
{
    class World;
}

class Player;
class Clock;

class Game
{
public:
    Game();
    ~Game();
    void Render() const;
    void Update();

    void HandleKeyBoardEvent(float deltaTime);
    void HandleMouseEvent(float deltaTime);
    // Game play logic
    void StartGame();

    // Camera
    void UpdateCameras(float deltaTime);

    // World
    void UpdateWorld();
    void RenderWorld() const;

    // Block Registration
    void RegisterBlocks();

public:
    std::unique_ptr<enigma::voxel::world::World> m_world;
    bool                                         m_enableChunkDebug = true;

public:
    bool m_isInMainMenu = true;
    bool m_isGameStart  = false;

    // Camera
    Camera* m_worldCamera  = nullptr;
    Camera* m_screenCamera = nullptr;

    // Space for both world and screen, camera needs them
    AABB2 m_screenSpace;
    AABB2 m_worldSpace;

    /// Clock
    Clock* m_clock = nullptr;
    /// 

    /// Player
    Player* m_player = nullptr;
    /// 

    /// Display Only
private:
#ifdef COSMIC
    float FluctuateValue(float value, float amplitude, float frequency, float deltaTime)
    {
        return value + amplitude * sinf(frequency * deltaTime);
    }

    float m_iconCircleRadius           = 200;
    float m_currentIconCircleThickness = 0.f;
    int   m_counter                    = 0;
#endif
};
