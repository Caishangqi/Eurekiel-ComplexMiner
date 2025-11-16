#pragma once
#include "../GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Framework/World/WorldConstant.hpp"

class Shader;
class ConstantBuffer;

namespace enigma::voxel
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
    void  UpdateWorld();
    void  RenderWorld() const;
    float GetTimeOfDay() const; // Get world time (0.0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk)
    Rgba8 CalculateSkyColor(float timeOfDay) const; // Calculate sky color
    Rgba8 CalculateOutdoorLightColor(float timeOfDay) const; // Calculate outdoor light color

    // Block Registration
    void RegisterBlocks();

public:
    std::unique_ptr<enigma::voxel::World> m_world;
    bool                                  m_enableChunkDebug = true;

    Shader*         m_worldShader = nullptr;
    ConstantBuffer* m_worldCBO    = nullptr;
    WorldConstant   cb_world;

    //Sky color interpolation system
    mutable Rgba8 m_skyColor = Rgba8(20, 20, 40); // Current sky color (default dark blue)

    // Outdoor light color interpolation system
    mutable Rgba8 m_outdoorLightColor = Rgba8(255, 255, 255); // Current outdoor light color (default pure white)

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
    Clock* m_clock      = nullptr;
    Clock* m_worldClock = nullptr; // World time clock (500:1 ratio)
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
