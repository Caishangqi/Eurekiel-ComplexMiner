#pragma once
#include "../../Framework/Entity/Entity.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/GameCommon.hpp"
#include <memory>

enum class CameraMode;
class GUIBlock3DSelection;
class GUIPlayerInventory;
class Camera;
class GameCamera;

class Player : public Entity
{
public:
    static bool Event_Player_Join_World(EventArgs& args);
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    Player(Game* owner);
    ~Player() override;

    // [NEW] Independent aim orientation (separate from Entity::m_orientation)
    // m_aim controls view direction, m_orientation controls physics/movement direction
    EulerAngles m_aim;

    // [NEW] Game camera system - manages 5 camera modes
    // Uses unique_ptr for automatic lifetime management
    std::unique_ptr<GameCamera> m_gameCamera;

    void Update(float deltaSeconds) override;
    void Render() const override;
    void RenderDebugPhysics() const; // [NEW] Physics debug visualization (Task 5.4)

    void ProcessInput(float deltaTime);

    // Camera mode accessor
    CameraMode GetCameraMode() const { return m_cameraMode; }

    // [NEW] Camera accessor - returns raw pointer (caller does not take ownership)
    GameCamera*       GetCamera() { return m_gameCamera.get(); }
    const GameCamera* GetCamera() const { return m_gameCamera.get(); }

    // [NEW] Aim accessor
    EulerAngles GetAim() const { return m_aim; }
    void        SetAim(const EulerAngles& aim) { m_aim = aim; }

private:
    // [REFACTORED] Phase 4.2 - Input handling methods
    void UpdateInput(float deltaSeconds); // Main input dispatcher
    void HandleCameraModeSwitch(); // C key - cycle camera modes
    void HandlePhysicsModeSwitch(); // V key - cycle physics modes
    void HandleMouseAndControllerInput(float deltaSeconds); // Mouse/controller view control
    void HandleMovementInput(float deltaSeconds); // WASD movement
    void HandleJumpInput(); // Space jump (WALKING mode only)

private:
    static std::shared_ptr<GUIPlayerInventory>  m_guiPlayerInventory;
    static std::shared_ptr<GUIBlock3DSelection> m_guiBlockSelection;

    CameraMode m_cameraMode = CameraMode::FIRST_PERSON;
};
