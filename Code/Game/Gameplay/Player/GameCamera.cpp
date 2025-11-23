#include "GameCamera.hpp"

#include "Player.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - Creates GameCamera with owned engine Camera via std::make_unique
//
GameCamera::GameCamera(Player* player)
    : m_cameraMode(CameraMode::FIRST_PERSON)
      , m_position(Vec3::ZERO)
      , m_orientation(EulerAngles())
      , m_engineCamera(std::make_unique<Camera>()) // [GOOD] Smart pointer manages lifetime
      , m_player(player)
      , m_spectatorVelocity(Vec3::ZERO)
      , m_movementSpeed(4.0f)
      , m_mouseSensitivity(0.075f)
      , m_deltaSeconds(0.0f)
{
    ASSERT_OR_DIE(m_player != nullptr, "GameCamera: player is null");
    ASSERT_OR_DIE(m_engineCamera != nullptr, "GameCamera: failed to create engine Camera");

    // Configure engine camera for perspective rendering
    // SetPerspectiveView(aspect, fov, near, far)
    constexpr float CAMERA_ASPECT_RATIO = 16.0f / 9.0f; // Widescreen aspect ratio
    constexpr float CAMERA_FOV_DEGREES  = 60.0f; // Field of view in degrees
    constexpr float CAMERA_NEAR_CLIP    = 0.01f; // Near clipping plane distance
    constexpr float CAMERA_FAR_CLIP     = 10000.0f; // Far clipping plane distance
    m_engineCamera->SetPerspectiveView(CAMERA_ASPECT_RATIO, CAMERA_FOV_DEGREES, CAMERA_NEAR_CLIP, CAMERA_FAR_CLIP);

    // Initialize position from player (will be updated on first frame)
    if (m_player)
    {
        m_position    = m_player->m_position + m_player->m_eyeOffset;
        m_orientation = m_player->m_aim;
    }
}

//-----------------------------------------------------------------------------------------------
// Destructor - unique_ptr automatically releases engine Camera
//
GameCamera::~GameCamera()
{
    // [GOOD] No manual deletion needed - std::unique_ptr handles cleanup automatically
    // m_player is not owned, so we don't delete it
}

//-----------------------------------------------------------------------------------------------
// Update - Main update loop, stores delta time and dispatches to mode-specific functions
//
void GameCamera::Update(float deltaSeconds)
{
    // Store deltaSeconds for mode update functions
    m_deltaSeconds = deltaSeconds;

    // Dispatch to mode-specific update function
    switch (m_cameraMode)
    {
    case CameraMode::FIRST_PERSON:
        UpdateFirstPerson();
        break;

    case CameraMode::OVER_SHOULDER:
        UpdateOverShoulder();
        break;

    case CameraMode::SPECTATOR:
        UpdateSpectator();
        break;

    case CameraMode::SPECTATOR_XY:
        UpdateSpectatorXY();
        break;

    case CameraMode::INDEPENDENT:
        UpdateIndependent();
        break;

    default:
        ERROR_AND_DIE("GameCamera::Update - Invalid camera mode");
        break;
    }
}

//-----------------------------------------------------------------------------------------------
// UpdateFromPlayer - Syncs camera with player state (called after player physics update)
//
void GameCamera::UpdateFromPlayer(float deltaSeconds)
{
    m_deltaSeconds = deltaSeconds;

    // For player-controlled modes, sync camera to player
    if (m_cameraMode == CameraMode::FIRST_PERSON || m_cameraMode == CameraMode::OVER_SHOULDER)
    {
        Update(deltaSeconds);
    }
    // For spectator/independent modes, camera is updated independently in Update()
}

//-----------------------------------------------------------------------------------------------
// NextCameraMode - Cycles to the next camera mode in sequence
//
void GameCamera::NextCameraMode()
{
    m_cameraMode = ::NextCameraMode(m_cameraMode); // Use global helper function from CameraMode.hpp

    // When switching to spectator modes, initialize spectator position from current camera
    if (m_cameraMode == CameraMode::SPECTATOR ||
        m_cameraMode == CameraMode::SPECTATOR_XY ||
        m_cameraMode == CameraMode::INDEPENDENT)
    {
        // Preserve current camera position/orientation for spectator modes
        // m_position and m_orientation already contain the last camera state
        m_spectatorVelocity = Vec3::ZERO; // Reset velocity on mode switch
    }
}

//-----------------------------------------------------------------------------------------------
// GetCameraModeName - Returns human-readable name for current camera mode
//
const char* GameCamera::GetCameraModeName() const
{
    return ::GetCameraModeName(m_cameraMode); // Use global helper function from CameraMode.hpp
}

//-----------------------------------------------------------------------------------------------
// UpdateFirstPerson - Camera at player eye position, using player aim
//
void GameCamera::UpdateFirstPerson()
{
    if (!m_player) return;

    // Camera position = player position + eye offset (typically ~1.65m up)
    m_position    = m_player->m_position + m_player->m_eyeOffset;
    m_orientation = m_player->m_aim;

    // Sync to engine camera
    m_engineCamera->SetPositionAndOrientation(m_position, m_orientation);
}

//-----------------------------------------------------------------------------------------------
// UpdateOverShoulder - Camera 4 meters behind player eye position
//
void GameCamera::UpdateOverShoulder()
{
    if (!m_player) return;

    Vec3 eyePos = m_player->m_position + m_player->m_eyeOffset;

    // Get forward vector from player aim
    Vec3 forward, left, up;
    m_player->m_aim.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

    // Position camera behind player eye position
    constexpr float OVER_SHOULDER_DISTANCE = 4.0f; // Distance behind player (meters)
    m_position                             = eyePos - forward * OVER_SHOULDER_DISTANCE;
    m_orientation                          = m_player->m_aim;

    // [TODO] Optional: Add raycast wall penetration check to prevent camera clipping through walls
    // VoxelRaycastResult3D result = world->RaycastWorld(eyePos, -forward, 4.0f);
    // if (result.m_didImpact) {
    //     m_position = result.m_impactPosition + result.m_impactNormal * 0.1f;
    // }

    // Sync to engine camera
    m_engineCamera->SetPositionAndOrientation(m_position, m_orientation);
}

//-----------------------------------------------------------------------------------------------
// UpdateSpectator - Free-flying spectator camera with WASD+QE movement
// Movement is relative to camera orientation (pressing W moves in camera forward direction)
//
void GameCamera::UpdateSpectator()
{
    // Process WASD+QE movement input (local space)
    Vec3 localMovement = Vec3::ZERO;

    if (g_theInput->IsKeyDown('W')) localMovement.x += 1.0f; // Forward
    if (g_theInput->IsKeyDown('S')) localMovement.x -= 1.0f; // Backward
    if (g_theInput->IsKeyDown('A')) localMovement.y += 1.0f; // Left (note: positive Y is left in our coordinate system)
    if (g_theInput->IsKeyDown('D')) localMovement.y -= 1.0f; // Right
    if (g_theInput->IsKeyDown('Q')) localMovement.z += 1.0f; // Up
    if (g_theInput->IsKeyDown('E')) localMovement.z -= 1.0f; // Down

    // Apply movement if any input detected
    if (localMovement.GetLengthSquared() > 0.0f)
    {
        localMovement = localMovement.GetNormalized();

        // Transform local movement to world space using camera orientation
        Mat44 transform     = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
        Vec3  worldMovement = transform.TransformVectorQuantity3D(localMovement);

        // Calculate speed (with Shift boost)
        constexpr float SPECTATOR_SPEED_BOOST = 20.0f; // Speed multiplier when Shift is held
        float           speed                 = m_movementSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
        {
            speed *= SPECTATOR_SPEED_BOOST;
        }

        // Update velocity and position
        m_spectatorVelocity = worldMovement * speed;
        m_position          += m_spectatorVelocity * m_deltaSeconds;
    }
    else
    {
        m_spectatorVelocity = Vec3::ZERO;
    }

    // Sync to engine camera
    m_engineCamera->SetPositionAndOrientation(m_position, m_orientation);
}

//-----------------------------------------------------------------------------------------------
// UpdateSpectatorXY - Spectator camera with WASD restricted to XY plane
// Q/E still provide independent vertical movement
//
void GameCamera::UpdateSpectatorXY()
{
    // Process WASD movement input (XY plane only)
    Vec3 localMovement = Vec3::ZERO;

    if (g_theInput->IsKeyDown('W')) localMovement.x += 1.0f; // Forward
    if (g_theInput->IsKeyDown('S')) localMovement.x -= 1.0f; // Backward
    if (g_theInput->IsKeyDown('A')) localMovement.y += 1.0f; // Left
    if (g_theInput->IsKeyDown('D')) localMovement.y -= 1.0f; // Right

    // Calculate world movement for XY plane
    Vec3 worldMovementXY = Vec3::ZERO;
    if (localMovement.GetLengthSquared() > 0.0f)
    {
        localMovement = localMovement.GetNormalized();

        // Transform local movement to world space
        Mat44 transform     = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
        Vec3  worldMovement = transform.TransformVectorQuantity3D(localMovement);

        // Project to XY plane (zero out Z component)
        worldMovementXY = Vec3(worldMovement.x, worldMovement.y, 0.0f);
        if (worldMovementXY.GetLengthSquared() > 0.0f)
        {
            worldMovementXY = worldMovementXY.GetNormalized();
        }
    }

    // Q/E for independent vertical movement
    float verticalMovement = 0.0f;
    if (g_theInput->IsKeyDown('Q')) verticalMovement += 1.0f; // Up
    if (g_theInput->IsKeyDown('E')) verticalMovement -= 1.0f; // Down

    // Calculate speed (with Shift boost)
    constexpr float SPECTATOR_SPEED_BOOST = 20.0f; // Speed multiplier when Shift is held
    float           speed                 = m_movementSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
    {
        speed *= SPECTATOR_SPEED_BOOST;
    }

    // Combine XY movement with vertical movement
    Vec3 finalMovement = worldMovementXY + Vec3(0.0f, 0.0f, verticalMovement);

    // Update velocity and position
    m_spectatorVelocity = finalMovement * speed;
    m_position          += m_spectatorVelocity * m_deltaSeconds;

    // Sync to engine camera
    m_engineCamera->SetPositionAndOrientation(m_position, m_orientation);
}

//-----------------------------------------------------------------------------------------------
// UpdateIndependent - Camera remains fixed, input controls player movement
// Camera position and orientation are not updated in this mode
//
void GameCamera::UpdateIndependent()
{
    // Camera maintains its current position and orientation
    // No changes to m_position or m_orientation
    // Player movement is handled by Player::HandleMovementInput()

    // Just sync current state to engine camera (position/orientation unchanged)
    m_engineCamera->SetPositionAndOrientation(m_position, m_orientation);
}

//-----------------------------------------------------------------------------------------------
// ProcessMouseInput - Process mouse movement input based on camera mode
//
void GameCamera::ProcessMouseInput(float deltaX, float deltaY)
{
    // Calculate yaw and pitch deltas from mouse movement
    float yawDelta   = deltaX * m_mouseSensitivity;
    float pitchDelta = deltaY * m_mouseSensitivity;

    // Update orientation based on camera mode
    if (m_cameraMode == CameraMode::FIRST_PERSON ||
        m_cameraMode == CameraMode::OVER_SHOULDER ||
        m_cameraMode == CameraMode::INDEPENDENT)
    {
        // Player-controlled modes: update player aim
        if (m_player)
        {
            m_player->m_aim.m_yawDegrees   += yawDelta;
            m_player->m_aim.m_pitchDegrees += pitchDelta;

            // Clamp pitch to prevent camera flip (gimbal lock)
            constexpr float PITCH_LIMIT_DEGREES = 85.0f; // Maximum pitch angle to prevent gimbal lock
            m_player->m_aim.m_pitchDegrees      = GetClamped(m_player->m_aim.m_pitchDegrees, -PITCH_LIMIT_DEGREES, PITCH_LIMIT_DEGREES);
        }
    }
    else
    {
        // Spectator modes: update camera orientation directly
        m_orientation.m_yawDegrees   += yawDelta;
        m_orientation.m_pitchDegrees += pitchDelta;

        // Clamp pitch to prevent camera flip (gimbal lock)
        constexpr float PITCH_LIMIT_DEGREES = 85.0f; // Maximum pitch angle to prevent gimbal lock
        m_orientation.m_pitchDegrees        = GetClamped(m_orientation.m_pitchDegrees, -PITCH_LIMIT_DEGREES, PITCH_LIMIT_DEGREES);
    }
}
