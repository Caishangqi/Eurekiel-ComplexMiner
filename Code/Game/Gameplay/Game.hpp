#pragma once
#include "../GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"

namespace enigma::voxel::world
{
    class World;
}

class Player;
class Clock;
class Prop;

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

    // Grid
    void RenderGrids() const;
    void RenderProps() const;

    // World
    void UpdateWorld();
    void RenderWorld() const;

    // Block Registration
    void InitializeBlocks();

private:
    void RenderEntities() const;
    void HandleEntityCollisions();
    void GarbageCollection();

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

    /// Cube
    Prop* m_cube   = nullptr;
    Prop* m_cube_1 = nullptr;
    ///

    /// Test Obj
    Prop* m_testProp = nullptr;
    /// 

    /// Balls
    Prop* m_ball = nullptr;
    /// 

    /// Grid
    Prop*              m_grid_x = nullptr;
    Prop*              m_grid_y = nullptr;
    std::vector<Prop*> m_grid_x_unit_5;
    std::vector<Prop*> m_grid_x_unit_1;
    std::vector<Prop*> m_grid_y_unit_5;
    std::vector<Prop*> m_grid_y_unit_1;
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
