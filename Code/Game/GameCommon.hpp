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


extern RandomNumberGenerator*               g_rng;
extern App*                                 g_theApp;
extern IRenderer*                           g_theRenderer;
extern InputSystem*                         g_theInput;
extern AudioSubsystem*                      g_theAudio;
extern Game*                                g_theGame;
extern GUISubsystem*                        g_theGUI;

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
/// 

/// Camera mode
enum class CameraMode
{
    SPECTATOR_FULL = 0,
    SPECTATOR_XY,
    THIRD_PERSON,
    FIRST_PERSON
};

const char* CameraModeToString(CameraMode mode);

void DebugDrawRing(const Vec2& center, float radius, float thickness, const Rgba8& color);

void DebugDrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba8& color);

void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& color);
void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& colorX, const Rgba8& colorNX, const Rgba8& colorY, const Rgba8& colorNY, const Rgba8& colorZ, const Rgba8& colorNZ);
