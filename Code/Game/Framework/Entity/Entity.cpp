#include "Entity.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Engine/Voxel/World/VoxelRaycastResult3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Framework/PhysicsConfigParser.hpp"

using namespace enigma::voxel;

Entity::Entity(Game* owner)
    : m_game(owner)
      , m_position(Vec3::ZERO)
      , m_velocity(Vec3::ZERO)
      , m_orientation()
      , m_scale(Vec3(1.0f, 1.0f, 1.0f))
      , m_angularVelocity()
      , m_color(Rgba8::WHITE)
      // [NEW] Physics extension (Task 1.1)
      , m_acceleration(Vec3::ZERO)
      , m_physicsBounds(Vec3(-0.3f, -0.3f, 0.0f), Vec3(0.3f, 0.3f, 1.8f)) // Default player size: 0.6m width x 1.8m height
      , m_physicsMode(PhysicsMode::WALKING)
      , m_isGrounded(false)
      // [NEW] Physics tuning parameters (loaded from settings.yml - Task 6.4)
      , m_gravityConstant(9.8f)
      , m_groundedDragCoefficient(8.0f)
      , m_airborneDragCoefficient(0.5f)
      , m_groundedAcceleration(10.0f)
      , m_airborneAcceleration(2.0f)
      , m_speedLimit(10.0f)
      , m_jumpImpulse(5.0f)
      , m_mouseSensitivity(0.075f)
      , m_eyeOffset(0.0f, 0.0f, 1.65f)
{
    // [NEW] Load physics parameters from settings.yml (Task 6.4)
    PhysicsConfig physicsConfig = PhysicsConfigParser::LoadFromYaml("Run/.enigma/settings.yml");
    m_gravityConstant           = physicsConfig.m_gravityConstant;
    m_groundedDragCoefficient   = physicsConfig.m_groundedDragCoefficient;
    m_airborneDragCoefficient   = physicsConfig.m_airborneDragCoefficient;
    m_groundedAcceleration      = physicsConfig.m_groundedAcceleration;
    m_airborneAcceleration      = physicsConfig.m_airborneAcceleration;
    m_speedLimit                = physicsConfig.m_speedLimit;
    m_jumpImpulse               = physicsConfig.m_jumpImpulse;
}

Entity::~Entity()
{
    m_game = nullptr;
}

void Entity::Update(float deltaSeconds)
{
    UpdatePhysics(deltaSeconds); // [NEW] Call physics update (Task 1.2)
    UpdateIsGrounded(); // [NEW] Update grounded state (Task 1.2)
}

//-----------------------------------------------------------------------------------------------
// [NEW] Physics update methods (Task 2.1)
// Implements core physics loop: gravity, friction, velocity integration, speed limiting
//-----------------------------------------------------------------------------------------------
void Entity::UpdatePhysics(float deltaSeconds)
{
    // 1. Calculate deltaPosition
    Vec3 deltaPosition = m_velocity * deltaSeconds;

    // [Early exit optimization] Skip complex calculations when nearly stationary
    if (deltaPosition.GetLengthSquared() < 0.0001f)
    {
        m_acceleration = Vec3::ZERO;
        return;
    }

    // 2. Apply gravity (only WALKING mode + not grounded + chunk exists under feet)
    if (m_physicsMode == PhysicsMode::WALKING && !m_isGrounded)
    {
        // Check if chunk exists under player's feet
        // Note: World access requires Game pointer and will be validated
        if (m_game != nullptr)
        {
            m_acceleration.z -= m_gravityConstant;
        }
    }

    // 3. Apply friction/drag (horizontal only)
    // [Task 2.4] Friction behavior differs by physics mode:
    // - WALKING: grounded=8.0, airborne=0.5
    // - FLYING: always airborne coefficient (0.5)
    // - NOCLIP: minimal friction (0.1) for responsive control
    Vec3  horizontalVelocity(m_velocity.x, m_velocity.y, 0.0f);
    float dragCoeff;
    if (m_physicsMode == PhysicsMode::NOCLIP)
    {
        dragCoeff = 0.1f; // Minimal friction for NOCLIP mode
    }
    else
    {
        dragCoeff = m_isGrounded ? m_groundedDragCoefficient : m_airborneDragCoefficient;
    }
    Vec3 drag      = -dragCoeff * horizontalVelocity;
    m_acceleration += drag;

    // 4. Velocity integration: velocity += acceleration * deltaSeconds
    m_velocity += m_acceleration * deltaSeconds;

    // 5. Limit horizontal speed (do not limit vertical speed for falling/jumping)
    Vec3  horizontalVel(m_velocity.x, m_velocity.y, 0.0f);
    float horizontalSpeed = horizontalVel.GetLength();
    if (horizontalSpeed > m_speedLimit)
    {
        float scale  = m_speedLimit / horizontalSpeed;
        m_velocity.x *= scale;
        m_velocity.y *= scale;
    }

    // 6. Collision detection (Task 2.2)
    if (m_physicsMode != PhysicsMode::NOCLIP)
    {
        ResolveCollisions(deltaPosition);
    }

    // 7. Apply final position
    m_position += deltaPosition;

    // 8. Reset acceleration for next frame
    m_acceleration = Vec3::ZERO;
}

//-----------------------------------------------------------------------------------------------
// [NEW] 4-Corner Grounded Detection (Task 2.3)
// Performs downward raycast from 4 bottom corners to detect if entity is standing on solid ground
// This system runs independently of collision detection and checks every frame
//-----------------------------------------------------------------------------------------------
void Entity::UpdateIsGrounded()
{
    // Non-WALKING modes do not need grounded detection
    if (m_physicsMode != PhysicsMode::WALKING)
    {
        m_isGrounded = false;
        return;
    }

    // Validate game and world pointers
    if (m_game == nullptr || g_theGame == nullptr || g_theGame->m_world == nullptr)
    {
        m_isGrounded = false;
        return;
    }

    // Build 4 bottom corner points (slightly above ground level to avoid floating point issues)
    // These are the same positions as the bottom layer of the 12-corner collision system
    float halfWidth = g_playerWidth * 0.5f - g_cornerOffset; // 0.29m (inset from edge)
    float bottomZ   = g_raycastOffset; // 0.01m (slightly above feet)

    Vec3 baseCorners[4] = {
        Vec3(-halfWidth, -halfWidth, bottomZ), // Back-left
        Vec3(+halfWidth, -halfWidth, bottomZ), // Back-right
        Vec3(+halfWidth, +halfWidth, bottomZ), // Front-right
        Vec3(-halfWidth, +halfWidth, bottomZ) // Front-left
    };

    // Ray parameters: always downward, fixed short distance
    Vec3  rayDirection(0.0f, 0.0f, -1.0f); // Always straight down (-Z)
    float rayDistance = 2.0f * g_raycastOffset; // Fixed 0.02m detection distance

    // Check each corner for ground contact
    m_isGrounded = false;
    for (int i = 0; i < 4; ++i)
    {
        // Transform corner to world space
        Vec3 rayStart = m_position + baseCorners[i];

        // Perform downward raycast
        VoxelRaycastResult3D result = g_theGame->m_world->RaycastVsBlocks(
            rayStart, rayDirection, rayDistance
        );

        // If any ray hits solid block, entity is grounded
        if (result.m_didImpact)
        {
            m_isGrounded = true;
            return; // Early exit: one hit is sufficient
        }
    }

    // No rays hit ground - entity is airborne
    // m_isGrounded remains false
}

void Entity::NextPhysicsMode()
{
    switch (m_physicsMode)
    {
    case PhysicsMode::WALKING:
        m_physicsMode = PhysicsMode::FLYING;
        break;
    case PhysicsMode::FLYING:
        m_physicsMode = PhysicsMode::NOCLIP;
        break;
    case PhysicsMode::NOCLIP:
        m_physicsMode = PhysicsMode::WALKING;
        break;
    default:
        m_physicsMode = PhysicsMode::WALKING;
        break;
    }
}

Mat44 Entity::GetModelToWorldTransform() const
{
    // Mat44::MakeNonUniformScale3D(m_scale);
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    matTranslation.Append(Mat44::MakeNonUniformScale3D(m_scale));
    return matTranslation;
}

//-----------------------------------------------------------------------------------------------
// [NEW] 12-Corner Collision Detection (Task 2.2)
// Build 12 corner points for collision detection: 3 layers (bottom, middle, top) x 4 corners each
//-----------------------------------------------------------------------------------------------
void Entity::BuildCornerPoints(Vec3 corners[12]) const
{
    // Half width with corner offset inset to avoid floating point precision issues
    float halfWidth = g_playerWidth * 0.5f - g_cornerOffset; // 0.3m - 0.01m = 0.29m

    // Three height levels for collision detection
    float bottomZ = g_cornerOffset; // Bottom layer: 0.01m (slightly above ground)
    float middleZ = g_playerHeight * 0.5f; // Middle layer: 0.9m (waist level)
    float topZ    = g_playerHeight - g_cornerOffset; // Top layer: 1.79m (slightly below head)

    // Bottom 4 corners (front-left, front-right, back-right, back-left)
    corners[0] = Vec3(-halfWidth, -halfWidth, bottomZ);
    corners[1] = Vec3(+halfWidth, -halfWidth, bottomZ);
    corners[2] = Vec3(+halfWidth, +halfWidth, bottomZ);
    corners[3] = Vec3(-halfWidth, +halfWidth, bottomZ);

    // Middle 4 corners
    corners[4] = Vec3(-halfWidth, -halfWidth, middleZ);
    corners[5] = Vec3(+halfWidth, -halfWidth, middleZ);
    corners[6] = Vec3(+halfWidth, +halfWidth, middleZ);
    corners[7] = Vec3(-halfWidth, +halfWidth, middleZ);

    // Top 4 corners
    corners[8]  = Vec3(-halfWidth, -halfWidth, topZ);
    corners[9]  = Vec3(+halfWidth, -halfWidth, topZ);
    corners[10] = Vec3(+halfWidth, +halfWidth, topZ);
    corners[11] = Vec3(-halfWidth, +halfWidth, topZ);
}

//-----------------------------------------------------------------------------------------------
// [NEW] Resolve Collisions using 12-corner raycast (Task 2.2)
// Performs raycast from each corner in the movement direction to detect and resolve collisions
//-----------------------------------------------------------------------------------------------
void Entity::ResolveCollisions(Vec3& deltaPosition)
{
    // [Early exit] Skip collision detection when stationary
    if (deltaPosition.GetLengthSquared() < 0.0001f)
    {
        return;
    }

    // Validate game and world pointers
    if (m_game == nullptr || g_theGame == nullptr || g_theGame->m_world == nullptr)
    {
        return;
    }

    // Build 12 corner points (3 layers x 4 corners)
    Vec3 corners[12];
    BuildCornerPoints(corners);

    // Calculate ray direction and distance
    Vec3  rayDirection = deltaPosition.GetNormalized();
    float rayDistance  = deltaPosition.GetLength() + g_raycastOffset;

    // Track closest impact fraction for each axis (1.0 = no impact)
    float closestImpactX = 1.0f;
    float closestImpactY = 1.0f;
    float closestImpactZ = 1.0f;

    // Raycast from each of the 12 corners
    for (int i = 0; i < 12; ++i)
    {
        // Transform corner to world space
        Vec3 worldCorner = m_position + corners[i];

        // Perform raycast against blocks
        VoxelRaycastResult3D result = g_theGame->m_world->RaycastVsBlocks(
            worldCorner, rayDirection, rayDistance
        );

        if (result.m_didImpact)
        {
            // Filter out back-face collisions (normal pointing same direction as ray)
            float normalDotDirection = DotProduct3D(result.m_impactNormal, rayDirection);
            if (normalDotDirection >= 0.0f)
            {
                continue; // Skip back-face hits
            }

            // Calculate impact fraction (0 to 1 range)
            float impactFraction = result.m_impactDist / rayDistance;

            // Record closest collision for each axis based on impact normal
            // Normal with magnitude > 0.5 indicates primary axis of collision
            if (fabsf(result.m_impactNormal.x) > 0.5f)
            {
                closestImpactX = (impactFraction < closestImpactX) ? impactFraction : closestImpactX;
            }
            if (fabsf(result.m_impactNormal.y) > 0.5f)
            {
                closestImpactY = (impactFraction < closestImpactY) ? impactFraction : closestImpactY;
            }
            if (fabsf(result.m_impactNormal.z) > 0.5f)
            {
                closestImpactZ = (impactFraction < closestImpactZ) ? impactFraction : closestImpactZ;
            }
        }
    }

    // Resolve collisions per-axis: zero out velocity and deltaPosition for blocked axes
    if (closestImpactX < 1.0f)
    {
        m_velocity.x    = 0.0f;
        deltaPosition.x = 0.0f;
    }
    if (closestImpactY < 1.0f)
    {
        m_velocity.y    = 0.0f;
        deltaPosition.y = 0.0f;
    }
    if (closestImpactZ < 1.0f)
    {
        m_velocity.z    = 0.0f;
        deltaPosition.z = 0.0f;
    }
}
