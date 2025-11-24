#pragma once
#include <vector>


/// Whether or not enable cosmic circle (developer)
#define COSMIC


namespace enigma::resource
{
    class ResourceSubsystem;
}

struct Vertex_PCU;
struct Rgba8;
struct Vec2;
class Camera;
class App;
class RandomNumberGenerator;
class IRenderer;
class InputSystem;
class AudioSubsystem;
class Game;
class GUISubsystem;


extern RandomNumberGenerator* g_rng;
extern App*                   g_theApp;
extern IRenderer*             g_theRenderer;
extern InputSystem*           g_theInput;
extern AudioSubsystem*        g_theAudio;
extern Game*                  g_theGame;
extern GUISubsystem*          g_theGUI;

// Debug Flags
extern bool g_debugPhysicsEnabled; // F3 key toggles physics debug rendering

constexpr float WORLD_SIZE_X   = 200.f;
constexpr float WORLD_SIZE_Y   = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
// Math
constexpr float PI = 3.14159265359f;
// Entity Data
constexpr int MAX_ENTITY_PER_TYPE = 64;

/// Grid
constexpr int GRID_SIZE      = 50; // Half
constexpr int GRID_UNIT_SIZE = 5;

//------------------------------------------------------------------------------------------------------------------------------
// Physics Constants - Used by Entity physics system (12-corner collision, 4-base grounded detection)
//------------------------------------------------------------------------------------------------------------------------------
constexpr float g_playerWidth   = 0.6f; // Player collision box width (meters)
constexpr float g_playerHeight  = 1.8f; // Player collision box height (meters)
constexpr float g_cornerOffset  = 0.1f; // Corner inset offset to avoid floating point precision issues (meters)
constexpr float g_raycastOffset = 0.2f; // Raycast offset to ensure accurate detection (meters)

//------------------------------------------------------------------------------------------------------------------------------
// [NEW] Anti-Tunneling Protection (Task 2.5)
// Fixed physics timestep ensures consistent collision detection at high speeds
//------------------------------------------------------------------------------------------------------------------------------
constexpr float g_fixedPhysicsTimeStep = 0.016f; // 60Hz physics update (1/60 seconds)
/// 

//------------------------------------------------------------------------------------------------------------------------------
// Camera Mode - Defined in CameraMode.hpp
//------------------------------------------------------------------------------------------------------------------------------
#include "Gameplay/Player/CameraMode.hpp"

void DebugDrawRing(const Vec2& center, float radius, float thickness, const Rgba8& color);

void DebugDrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba8& color);

void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& color);
void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& colorX, const Rgba8& colorNX, const Rgba8& colorY, const Rgba8& colorNY, const Rgba8& colorZ, const Rgba8& colorNZ);
