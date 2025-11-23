#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB3.hpp"
#include "PhysicsMode.hpp"

class Game;

class Entity
{
public:
    Entity(Game* owner);
    virtual ~Entity();

    virtual void Update(float deltaSeconds);
    virtual void Render() const = 0;

    virtual Mat44 GetModelToWorldTransform() const;

    //-----------------------------------------------------------------------------------------------
    // [NEW] Physics update methods (Task 1.2)
    //-----------------------------------------------------------------------------------------------
    void UpdatePhysics(float deltaSeconds);
    void UpdateIsGrounded();

    //-----------------------------------------------------------------------------------------------
    // [NEW] Physics state accessors (Task 1.2)
    //-----------------------------------------------------------------------------------------------
    bool        IsGrounded() const { return m_isGrounded; }
    PhysicsMode GetPhysicsMode() const { return m_physicsMode; }
    void        SetPhysicsMode(PhysicsMode mode) { m_physicsMode = mode; }
    void        NextPhysicsMode();

    //-----------------------------------------------------------------------------------------------
    // [EXISTING] Core transform members
    //-----------------------------------------------------------------------------------------------
    Game*       m_game = nullptr;
    Vec3        m_position;
    Vec3        m_velocity;
    EulerAngles m_orientation;
    Vec3        m_scale = Vec3(1, 1, 1);
    EulerAngles m_angularVelocity;

    Rgba8 m_color = Rgba8::WHITE;

    //-----------------------------------------------------------------------------------------------
    // [NEW] Physics extension members (Task 1.1)
    //-----------------------------------------------------------------------------------------------
    Vec3        m_acceleration; // Per-frame accumulated acceleration
    AABB3       m_physicsBounds; // Local-space collision bounding box
    PhysicsMode m_physicsMode = PhysicsMode::WALKING; // Physics mode (WALKING/FLYING/NOCLIP)
    bool        m_isGrounded  = false; // Grounded state

    //-----------------------------------------------------------------------------------------------
    // [NEW] Physics tuning parameters (loaded from settings.yml)
    //-----------------------------------------------------------------------------------------------
    float m_gravityConstant         = 9.8f; // Gravity acceleration (m/s^2)
    float m_groundedDragCoefficient = 8.0f; // Grounded friction coefficient
    float m_airborneDragCoefficient = 0.5f; // Airborne friction coefficient
    float m_groundedAcceleration    = 10.0f; // Grounded acceleration
    float m_airborneAcceleration    = 2.0f; // Airborne acceleration
    float m_speedLimit              = 10.0f; // Maximum horizontal speed (m/s)
    float m_jumpImpulse             = 5.0f; // Jump instant velocity (m/s)
    float m_mouseSensitivity        = 0.075f; // Mouse sensitivity
    Vec3  m_eyeOffset               = Vec3(0.0f, 0.0f, 1.65f); // Eye position offset from feet

    //-----------------------------------------------------------------------------------------------
    // [NEW] Anti-Tunneling Protection (Task 2.5)
    // Physics accumulator for fixed timestep simulation
    //-----------------------------------------------------------------------------------------------
    float m_physicsAccumulator = 0.0f; // Accumulated time for fixed physics updates

protected:
    //-----------------------------------------------------------------------------------------------
    // [NEW] Private collision detection methods (Task 2.2)
    //-----------------------------------------------------------------------------------------------
    void ResolveCollisions(Vec3& deltaPosition);
    void BuildCornerPoints(Vec3 corners[12]) const;
};
