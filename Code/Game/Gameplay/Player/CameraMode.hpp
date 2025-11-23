#pragma once

//-----------------------------------------------------------------------------------------------
// CameraMode.hpp
// Defines camera mode enumeration and helper functions for GameCamera view management.
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Camera mode enumeration
// Controls how GameCamera positions and orients relative to Player.
//
// Control Target Summary:
// - FIRST_PERSON, OVER_SHOULDER: Input controls Player, camera follows
// - SPECTATOR, SPECTATOR_XY: Input controls Camera directly (Player dispossessed)
// - INDEPENDENT: Input controls Player, camera stays fixed
//-----------------------------------------------------------------------------------------------
enum class CameraMode
{
    FIRST_PERSON, // First-person: Camera at player eye position, player not rendered
    OVER_SHOULDER, // Over-shoulder: Camera behind player (4m offset), may need anti-wall-clip
    SPECTATOR, // Spectator: Player dispossessed, WASD moves camera relative to camera orientation
    SPECTATOR_XY, // Spectator XY: WASD restricted to XY plane, Q/E for vertical movement
    INDEPENDENT // Independent: Camera fixed, input controls player movement
};

//-----------------------------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Returns a human-readable name for the given camera mode.
// Includes control target hint in parentheses for clarity.
// Used for HUD display and debugging.
//-----------------------------------------------------------------------------------------------
inline const char* GetCameraModeName(CameraMode mode)
{
    switch (mode)
    {
    case CameraMode::FIRST_PERSON: return "FIRST_PERSON (Player)";
    case CameraMode::OVER_SHOULDER: return "OVER_SHOULDER (Player)";
    case CameraMode::SPECTATOR: return "SPECTATOR (Camera)";
    case CameraMode::SPECTATOR_XY: return "SPECTATOR_XY (Camera)";
    case CameraMode::INDEPENDENT: return "INDEPENDENT (Player)";
    default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------------------------
// Returns the next camera mode in the cycle:
// FIRST_PERSON -> OVER_SHOULDER -> SPECTATOR -> SPECTATOR_XY -> INDEPENDENT -> FIRST_PERSON
// Used for C key cycling in Player input handling.
//-----------------------------------------------------------------------------------------------
inline CameraMode NextCameraMode(CameraMode current)
{
    switch (current)
    {
    case CameraMode::FIRST_PERSON: return CameraMode::OVER_SHOULDER;
    case CameraMode::OVER_SHOULDER: return CameraMode::SPECTATOR;
    case CameraMode::SPECTATOR: return CameraMode::SPECTATOR_XY;
    case CameraMode::SPECTATOR_XY: return CameraMode::INDEPENDENT;
    case CameraMode::INDEPENDENT: return CameraMode::FIRST_PERSON;
    default: return CameraMode::FIRST_PERSON;
    }
}
