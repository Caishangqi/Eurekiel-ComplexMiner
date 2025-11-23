#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "CameraMode.hpp"
#include <memory>

// Forward declarations
class Camera;
class Player;

//-----------------------------------------------------------------------------------------------
// GameCamera - High-level camera controller for SimpleMiner
// Wraps the low-level Engine Camera class and provides game-specific camera modes.
//
// Design Notes:
// - Uses std::unique_ptr to manage engine Camera lifecycle [IMPORTANT]
// - Supports 5 camera modes with different control behaviors
// - m_position and m_orientation store the current camera world-space state
// - m_spectatorVelocity stores velocity for spectator mode smooth movement
//
// Camera Mode Control Summary:
// - FIRST_PERSON, OVER_SHOULDER: Input controls Player, camera follows player
// - SPECTATOR, SPECTATOR_XY: Input controls Camera directly (Player dispossessed)
// - INDEPENDENT: Input controls Player, camera stays fixed at last position
//-----------------------------------------------------------------------------------------------
class GameCamera
{
public:
    //-------------------------------------------------------------------------------------------
    // Constructor & Destructor
    //-------------------------------------------------------------------------------------------

    /// Constructs GameCamera and creates engine Camera via std::make_unique
    /// @param player Pointer to the associated player (not owned, must outlive GameCamera)
    explicit GameCamera(Player* player);

    /// Destructor - unique_ptr automatically releases engine Camera
    ~GameCamera();

    //-------------------------------------------------------------------------------------------
    // Core Update Methods
    //-------------------------------------------------------------------------------------------

    /// Main update loop - dispatches to mode-specific update functions
    /// @param deltaSeconds Frame delta time in seconds
    void Update(float deltaSeconds);

    /// Updates camera state based on player position and aim
    /// Called after player physics update to sync camera with player
    /// @param deltaSeconds Frame delta time in seconds
    void UpdateFromPlayer(float deltaSeconds);

    /// Processes mouse input for camera orientation
    /// Updates player aim (FIRST_PERSON/OVER_SHOULDER) or spectator orientation (SPECTATOR modes)
    /// @param deltaX Mouse X delta in pixels
    /// @param deltaY Mouse Y delta in pixels
    void ProcessMouseInput(float deltaX, float deltaY);

    //-------------------------------------------------------------------------------------------
    // Camera Mode Management
    //-------------------------------------------------------------------------------------------

    /// Gets the current camera mode
    CameraMode GetCameraMode() const { return m_cameraMode; }

    /// Sets the camera mode directly
    /// @param mode New camera mode to set
    void SetCameraMode(CameraMode mode) { m_cameraMode = mode; }

    /// Cycles to the next camera mode in sequence
    /// Order: FIRST_PERSON -> OVER_SHOULDER -> SPECTATOR -> SPECTATOR_XY -> INDEPENDENT -> ...
    void NextCameraMode();

    /// Gets a human-readable name for the current camera mode
    /// @return String name of current mode (e.g., "FIRST_PERSON (Player)")
    const char* GetCameraModeName() const;

    //-------------------------------------------------------------------------------------------
    // Transform Accessors
    //-------------------------------------------------------------------------------------------

    /// Gets the current camera world position
    Vec3 GetPosition() const { return m_position; }

    /// Gets the current camera orientation
    EulerAngles GetOrientation() const { return m_orientation; }

    /// Sets the camera position directly (used for spectator modes)
    void SetPosition(const Vec3& position) { m_position = position; }

    /// Sets the camera orientation directly (used for spectator modes)
    void SetOrientation(const EulerAngles& orientation) { m_orientation = orientation; }

    //-------------------------------------------------------------------------------------------
    // Engine Camera Access
    //-------------------------------------------------------------------------------------------

    /// Gets the underlying engine Camera for rendering
    /// [GOOD] Returns raw pointer - caller does not take ownership
    Camera* GetEngineCamera() { return m_engineCamera.get(); }

    /// Gets the underlying engine Camera for rendering (const version)
    const Camera* GetEngineCamera() const { return m_engineCamera.get(); }

private:
    //-------------------------------------------------------------------------------------------
    // Mode-Specific Update Functions
    //-------------------------------------------------------------------------------------------

    /// Updates camera for first-person view (camera at player eye position)
    void UpdateFirstPerson();

    /// Updates camera for over-shoulder view (camera 4m behind player)
    void UpdateOverShoulder();

    /// Updates camera for spectator mode (free-flying, WASD relative to camera orientation)
    void UpdateSpectator();

    /// Updates camera for spectator XY mode (WASD restricted to XY plane)
    void UpdateSpectatorXY();

    /// Updates camera for independent mode (camera fixed, input controls player)
    void UpdateIndependent();

private:
    //-------------------------------------------------------------------------------------------
    // Member Variables
    //-------------------------------------------------------------------------------------------

    // Camera mode
    CameraMode m_cameraMode = CameraMode::FIRST_PERSON;

    // Camera world-space state
    Vec3        m_position    = Vec3::ZERO; // Current camera position
    EulerAngles m_orientation = EulerAngles(); // Current camera orientation (yaw, pitch, roll)

    // Engine Camera - managed via unique_ptr [IMPORTANT]
    std::unique_ptr<Camera> m_engineCamera;

    // Associated player (not owned, must outlive GameCamera)
    Player* m_player = nullptr;

    // Spectator mode velocity (for smooth movement)
    Vec3 m_spectatorVelocity = Vec3::ZERO;

    // Configuration
    float m_movementSpeed    = 4.0f; // Base movement speed (units/second)
    float m_mouseSensitivity = 0.075f; // Mouse sensitivity (degrees/pixel)

    // Frame timing cache (set by Update, used by mode update functions)
    float m_deltaSeconds = 0.0f;
};
