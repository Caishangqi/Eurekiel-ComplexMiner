#pragma once

//-----------------------------------------------------------------------------------------------
// PhysicsMode.hpp
// Defines physics mode enumeration and helper functions for Entity physics simulation.
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Physics mode enumeration
// Controls how Entity physics behaves (gravity, collision, grounding)
//-----------------------------------------------------------------------------------------------
enum class PhysicsMode
{
    WALKING, // Full physics: gravity, collision, grounding
    FLYING, // No gravity: collision enabled, no grounding
    NOCLIP // Ghost mode: no gravity, no collision
};

//-----------------------------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Returns a human-readable name for the given physics mode.
// Used for HUD display and debugging.
//-----------------------------------------------------------------------------------------------
inline const char* GetPhysicsModeName(PhysicsMode mode)
{
    switch (mode)
    {
    case PhysicsMode::WALKING: return "WALKING";
    case PhysicsMode::FLYING: return "FLYING";
    case PhysicsMode::NOCLIP: return "NOCLIP";
    default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------------------------
// Returns the next physics mode in the cycle: WALKING -> FLYING -> NOCLIP -> WALKING
// Used for V key cycling in Player input handling.
//-----------------------------------------------------------------------------------------------
inline PhysicsMode NextPhysicsMode(PhysicsMode current)
{
    switch (current)
    {
    case PhysicsMode::WALKING: return PhysicsMode::FLYING;
    case PhysicsMode::FLYING: return PhysicsMode::NOCLIP;
    case PhysicsMode::NOCLIP: return PhysicsMode::WALKING;
    default: return PhysicsMode::WALKING;
    }
}
