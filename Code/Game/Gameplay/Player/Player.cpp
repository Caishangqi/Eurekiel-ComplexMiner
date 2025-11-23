#include "Player.hpp"
#include "GameCamera.hpp"

#include "../../GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Voxel/Builtin/DefaultBlock.hpp"
#include <cmath>

#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Framework/GUISubsystem.hpp"
#include "Game/Framework/ControlConfigParser.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/gui/GUIBlock3DSelection.hpp"
#include "Game/Gameplay/gui/GUICrosser.hpp"
#include "Game/Gameplay/gui/GUIPlayerInventory.hpp"
#include "Game/Gameplay/gui/GUIPlayerStats.hpp"

std::shared_ptr<GUIPlayerInventory>  Player::m_guiPlayerInventory = nullptr;
std::shared_ptr<GUIBlock3DSelection> Player::m_guiBlockSelection  = nullptr;

bool Player::Event_Player_Join_World(EventArgs& args)
{
    UNUSED(args)
    std::shared_ptr<GUI> gui = g_theGUI->GetGUI(std::type_index(typeid(GUICrosser)));
    if (!gui)
    {
        g_theGUI->AddToViewPort(std::make_shared<GUICrosser>(g_theGame->m_player));
    }

    // [NEW] Add GUIPlayerStats to display camera mode, physics mode, and FPS
    std::shared_ptr<GUI> guiPlayerStats = g_theGUI->GetGUI(std::type_index(typeid(GUIPlayerStats)));
    if (!guiPlayerStats)
    {
        g_theGUI->AddToViewPort(std::make_unique<GUIPlayerStats>());
    }

    std::shared_ptr<GUI> guiInventory = g_theGUI->GetGUI(std::type_index(typeid(GUIPlayerInventory)));
    if (!gui)
    {
        m_guiPlayerInventory = std::dynamic_pointer_cast<GUIPlayerInventory>(g_theGUI->AddToViewPort(std::make_shared<GUIPlayerInventory>()));
    }
    std::shared_ptr<GUI> guiBlockSelection = g_theGUI->GetGUI(std::type_index(typeid(GUIBlock3DSelection)));
    if (!gui)
    {
        m_guiBlockSelection = std::dynamic_pointer_cast<GUIBlock3DSelection>(g_theGUI->AddToViewPort(std::make_shared<GUIBlock3DSelection>()));
    }
    return false;
}

bool Player::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)
    std::shared_ptr<GUI> gui = g_theGUI->GetGUI(std::type_index(typeid(GUICrosser)));
    if (gui)
        g_theGUI->RemoveFromViewPort(gui);
    return false;
}

Player::Player(Game* owner) : Entity(owner)
{
    // [NEW] Initialize m_aim from m_orientation (sync initial view direction)
    m_aim = m_orientation;

    // [NEW] Create GameCamera with unique_ptr - automatic lifetime management
    m_gameCamera = std::make_unique<GameCamera>(this);

    // [NEW] Load control parameters from settings.yml (Task 6.4)
    ControlConfig controlConfig = ControlConfigParser::LoadFromYaml("Run/.enigma/settings.yml");
    m_mouseSensitivity          = controlConfig.m_mouseSensitivity;

    // Event Subscribe
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerJoinWorld", Event_Player_Join_World);
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
}

Player::~Player()
{
    // [NEW] m_gameCamera uses unique_ptr - automatically cleaned up
    // No need to explicitly delete
}

void Player::Update(float deltaSeconds)
{
    // [REFACTORED] Phase 4.2 - New update flow:
    // 1. UpdateInput: Handle all input (camera mode switch, physics mode switch, mouse, movement, jump)
    // 2. Entity::Update: Physics simulation (calls UpdatePhysics and UpdateIsGrounded internally)
    // 3. GameCamera::UpdateFromPlayer: Sync camera with player state

    UpdateInput(deltaSeconds);
    Entity::Update(deltaSeconds);
    m_gameCamera->UpdateFromPlayer(deltaSeconds);

    // [DEBUG] Log camera state
    Vec3        camPos    = m_gameCamera->GetEngineCamera()->GetPosition();
    EulerAngles camOrient = m_gameCamera->GetEngineCamera()->GetOrientation();
    DebuggerPrintf("[DEBUG] Camera Pos: (%.2f, %.2f, %.2f) Orient: (%.2f, %.2f, %.2f)\n",
                   camPos.x, camPos.y, camPos.z, camOrient.m_yawDegrees, camOrient.m_pitchDegrees, camOrient.m_rollDegrees);
    DebuggerPrintf("[DEBUG] Player Pos: (%.2f, %.2f, %.2f) Aim: (%.2f, %.2f, %.2f)\n",
                   m_position.x, m_position.y, m_position.z, m_aim.m_yawDegrees, m_aim.m_pitchDegrees, m_aim.m_rollDegrees);
}

void Player::UpdateInput(float deltaSeconds)
{
    // 1. Handle mode switches
    HandleCameraModeSwitch(); // C key - cycle camera modes
    HandlePhysicsModeSwitch(); // V key - cycle physics modes

    // [NEW] F3 key - toggle physics debug rendering (Task 5.4)
    if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
    {
        g_debugPhysicsEnabled = !g_debugPhysicsEnabled;
    }

    // 2. Handle mouse/controller input for view direction
    HandleMouseAndControllerInput(deltaSeconds);

    // 3. Handle movement based on camera mode
    CameraMode cameraMode = m_gameCamera->GetCameraMode();

    if (cameraMode == CameraMode::SPECTATOR || cameraMode == CameraMode::SPECTATOR_XY)
    {
        // Spectator modes: Camera handles its own movement in GameCamera::Update*()
        // Player movement is disabled - player is "dispossessed"
        // GameCamera::Update() is called from UpdateFromPlayer() for these modes
        m_gameCamera->Update(deltaSeconds);
    }
    else if (cameraMode == CameraMode::INDEPENDENT)
    {
        // Independent mode: Input controls player, camera stays fixed
        HandleMovementInput(deltaSeconds);
        HandleJumpInput();
    }
    else
    {
        // FIRST_PERSON, OVER_SHOULDER: Standard player control
        HandleMovementInput(deltaSeconds);
        HandleJumpInput();
    }

    // 4. Process block interaction input (dig, place)
    ProcessInput(deltaSeconds);
}

void Player::Render() const
{
    g_theRenderer->BeginCamera(*m_gameCamera->GetEngineCamera());

    // [FIX] Ensure camera context is always properly closed
    if (g_debugPhysicsEnabled && m_gameCamera->GetCameraMode() != CameraMode::FIRST_PERSON)
    {
        RenderDebugPhysics();
    }

    g_theRenderer->EndCamera(*m_gameCamera->GetEngineCamera());
}

void Player::ProcessInput(float deltaTime)
{
    UNUSED(deltaTime)

    // [NEW] Block interaction - based on ray detection results
    if (!m_guiBlockSelection)
    {
        return; // No ray detection GUI, skip interaction
    }

    // Get the current ray detection results
    const enigma::voxel::VoxelRaycastResult3D& raycast = m_guiBlockSelection->GetCurrentRaycast();

    // LMB - Dig Block
    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE))
    {
        if (raycast.m_didImpact && m_game->m_world)
        {
            // Call World::DigBlock to mine the hit block
            m_game->m_world->DigBlock(raycast.m_hitBlockIter);
        }
    }

    // RMB - Place Block
    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_RIGHT_MOUSE))
    {
        if (raycast.m_didImpact && m_guiPlayerInventory && m_game->m_world)
        {
            // Get the currently selected block type
            auto selectedBlock = m_guiPlayerInventory->GetCurrentBlock();
            if (selectedBlock)
            {
                // Place on the opposite side of the hit surface (using GetPlacementIterator)
                enigma::voxel::BlockIterator placeIter = raycast.GetPlacementIterator();
                if (placeIter.IsValid())
                {
                    // Call World::PlaceBlock to place the block
                    m_game->m_world->PlaceBlock(placeIter, selectedBlock->GetDefaultState());
                }
            }
        }
    }
}

void Player::HandleCameraModeSwitch()
{
    // C key - cycle through camera modes via GameCamera
    if (g_theInput->WasKeyJustPressed('C'))
    {
        m_gameCamera->NextCameraMode();

        // [LEGACY] Keep m_cameraMode in sync for backward compatibility
        // This will be removed after full migration
        m_cameraMode = m_gameCamera->GetCameraMode();
    }
}

void Player::HandlePhysicsModeSwitch()
{
    // V key - cycle through physics modes (WALKING -> FLYING -> NOCLIP -> WALKING)
    if (g_theInput->WasKeyJustPressed('V'))
    {
        NextPhysicsMode();
    }
}

void Player::HandleJumpInput()
{
    // Space key - jump (only in WALKING mode when grounded)
    if (m_physicsMode == PhysicsMode::WALKING &&
        m_isGrounded &&
        g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
    {
        // Apply jump impulse to vertical velocity
        m_velocity.z += m_jumpImpulse;
    }
}

void Player::HandleMouseAndControllerInput(float deltaSeconds)
{
    // [REFACTORED] Phase 4.4 - Route mouse input through GameCamera
    // GameCamera::ProcessMouseInput handles updating m_aim (player modes) or camera orientation (spectator modes)

    // Mouse input
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
    m_gameCamera->ProcessMouseInput(-cursorDelta.x, -cursorDelta.y);

    // Controller right stick input for view direction
    const XboxController& controller    = g_theInput->GetController(0);
    Vec2                  rightStickPos = controller.GetRightStick().GetPosition();
    float                 rightStickMag = controller.GetRightStick().GetMagnitude();
    float                 speed         = 4.0f;

    if (rightStickMag > 0.f)
    {
        float controllerYaw   = -(rightStickPos * speed * rightStickMag).x;
        float controllerPitch = -(rightStickPos * speed * rightStickMag).y;
        m_gameCamera->ProcessMouseInput(controllerYaw, controllerPitch);
    }

    // Controller trigger input for roll (legacy - only affects m_orientation)
    float leftTrigger           = controller.GetLeftTrigger();
    float rightTrigger          = controller.GetRightTrigger();
    m_orientation.m_rollDegrees += leftTrigger * 0.125f * deltaSeconds * speed;
    m_orientation.m_rollDegrees -= rightTrigger * 0.125f * deltaSeconds * speed;

    // Clamp roll
    m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
}

void Player::HandleMovementInput(float deltaSeconds)
{
    // [REFACTORED] Phase 4.3 - Movement based on m_aim, sets acceleration for physics system
    UNUSED(deltaSeconds)

    // Build local movement input vector
    Vec3 movementInput = Vec3::ZERO;
    if (g_theInput->IsKeyDown('W')) movementInput.x += 1.0f; // Forward
    if (g_theInput->IsKeyDown('S')) movementInput.x -= 1.0f; // Backward
    if (g_theInput->IsKeyDown('A')) movementInput.y += 1.0f; // Left
    if (g_theInput->IsKeyDown('D')) movementInput.y -= 1.0f; // Right

    // Controller left stick input
    const XboxController& controller   = g_theInput->GetController(0);
    Vec2                  leftStickPos = controller.GetLeftStick().GetPosition();
    float                 leftStickMag = controller.GetLeftStick().GetMagnitude();
    if (leftStickMag > 0.0f)
    {
        movementInput.x += leftStickPos.y * leftStickMag; // Y axis is forward/back
        movementInput.y -= leftStickPos.x * leftStickMag; // X axis is left/right (inverted)
    }

    // Normalize to prevent diagonal speed boost
    if (movementInput.GetLengthSquared() > 0.0f)
    {
        movementInput = movementInput.GetNormalized();
    }

    // Transform local movement to world space using m_aim
    Vec3 forward, left, up;
    m_aim.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
    Vec3 worldMovement = forward * movementInput.x + left * movementInput.y;

    // WALKING mode: restrict movement to XY plane
    if (m_physicsMode == PhysicsMode::WALKING)
    {
        worldMovement.z = 0.0f;
        if (worldMovement.GetLengthSquared() > 0.0f)
        {
            worldMovement = worldMovement.GetNormalized();
        }
    }

    // Q/E vertical movement (only in FLYING/NOCLIP modes, or non-first-person camera)
    CameraMode cameraMode = m_gameCamera->GetCameraMode();
    if (m_physicsMode != PhysicsMode::WALKING || cameraMode != CameraMode::FIRST_PERSON)
    {
        if (g_theInput->IsKeyDown('Q') || controller.IsButtonDown(XBOX_BUTTON_LS))
            worldMovement.z += 1.0f; // Up
        if (g_theInput->IsKeyDown('E') || controller.IsButtonDown(XBOX_BUTTON_RS))
            worldMovement.z -= 1.0f; // Down
    }

    // Calculate sprint modifier
    float sprintMod = 1.0f;
    if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A))
    {
        sprintMod = 20.0f;
    }

    // Calculate acceleration based on grounded state
    float dragCoeff     = m_isGrounded ? m_groundedDragCoefficient : m_airborneDragCoefficient;
    float accelConstant = m_isGrounded ? m_groundedAcceleration : m_airborneAcceleration;

    // Apply acceleration to physics system (will be integrated in Entity::UpdatePhysics)
    m_acceleration += worldMovement * sprintMod * dragCoeff * accelConstant;
}


void Player::RenderDebugPhysics() const
{
    std::vector<Vertex_PCU> debugVerts;
    debugVerts.reserve(1024);

    // Early exit if debug rendering is disabled
    if (!g_debugPhysicsEnabled)
        return;

    // [IMPORTANT] Only render in non-first-person modes to avoid obstructing view
    if (m_gameCamera->GetCameraMode() == CameraMode::FIRST_PERSON)
        return;

    // 1. Draw player bounding box (cyan, X-Ray mode)
    AABB3 worldBounds  = m_physicsBounds;
    worldBounds.m_mins += m_position;
    worldBounds.m_maxs += m_position;
    AddVertsForCube3DWireFrame(debugVerts, worldBounds, Rgba8::CYAN);

    // 2. Draw 12 collision detection rays (only when moving)
    if (m_velocity.GetLengthSquared() > 0.0001f)
    {
        Vec3 corners[12];
        BuildCornerPoints(corners);

        Vec3  rayDirection = m_velocity.GetNormalized();
        float deltaTime    = g_theGame->m_clock->GetDeltaSeconds();
        float rayDistance  = m_velocity.GetLength() * deltaTime + g_raycastOffset;

        for (int i = 0; i < 12; ++i)
        {
            Vec3 rayStart = m_position + corners[i];
            Vec3 rayEnd   = rayStart + rayDirection * rayDistance;
            AddVertsForArrow3D(debugVerts, rayEnd, rayStart, 0.05f, 0.1f, Rgba8::CYAN);
        }
    }

    // 3. Draw 4 ground detection rays (green if grounded, red if not)
    Vec3  baseCorners[4];
    float halfWidth = g_playerWidth * 0.5f - g_cornerOffset;

    baseCorners[0] = Vec3(-halfWidth, -halfWidth, g_raycastOffset);
    baseCorners[1] = Vec3(+halfWidth, -halfWidth, g_raycastOffset);
    baseCorners[2] = Vec3(+halfWidth, +halfWidth, g_raycastOffset);
    baseCorners[3] = Vec3(-halfWidth, +halfWidth, g_raycastOffset);

    Rgba8 groundRayColor = m_isGrounded ? Rgba8::GREEN : Rgba8::RED;
    for (int i = 0; i < 4; ++i)
    {
        Vec3 rayStart = m_position + baseCorners[i];
        Vec3 rayEnd   = rayStart + Vec3(0.0f, 0.0f, -2.0f * g_raycastOffset);
        AddVertsForArrow3D(debugVerts, rayStart, rayEnd, 0.05f, 0.1f, groundRayColor, 6);
    }
}
