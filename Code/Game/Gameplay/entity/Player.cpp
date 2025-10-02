#include "Player.hpp"

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
#include "Engine/Window/Window.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Framework/GUISubsystem.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/gui/GUIBlock3DSelection.hpp"
#include "Game/Gameplay/gui/GUICrosser.hpp"
#include "Game/Gameplay/gui/GUIPlayerInventory.hpp"

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
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));

    // Event Subscribe
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerJoinWorld", Event_Player_Join_World);
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
}

Player::~Player()
{
    POINTER_SAFE_DELETE(m_camera)
}

void Player::Update(float deltaSeconds)
{
    HandleCameraModeSwitch();
    HandleMouseAndControllerInput(deltaSeconds);
    HandleMovementInput(deltaSeconds);
    UpdateCameraSettings();

    // Input
    ProcessInput(deltaSeconds);
    Entity::Update(deltaSeconds);
}

void Player::Render() const
{
    g_theRenderer->BeginCamera(*m_camera);
    g_theRenderer->EndCamera(*m_camera);
}

void Player::ProcessInput(float deltaTime)
{
    UNUSED(deltaTime)
    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE))
    {
        // Place Current Block
        if (m_guiPlayerInventory)
        {
            auto block = m_guiPlayerInventory->GetCurrentBlock();
            if (block != nullptr && block != enigma::voxel::block::AIR)
            {
                // Use std::floor to correctly convert floating point coordinates to square coordinates
                int32_t blockX = static_cast<int32_t>(std::floor(m_position.x));
                int32_t blockY = static_cast<int32_t>(std::floor(m_position.y));
                int32_t blockZ = static_cast<int32_t>(std::floor(m_position.z));

                auto topBlockZ     = m_game->m_world->GetTopBlockZ(enigma::voxel::block::BlockPos(blockX, blockY, blockZ));
                auto blockPlacePos = enigma::voxel::block::BlockPos(blockX, blockY, topBlockZ + 1);
                m_game->m_world->SetBlockState(blockPlacePos, block->GetDefaultState());
            }
        }
    }

    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_RIGHT_MOUSE))
    {
        // Destroy Block (replace with air)
        if (m_game->m_world)
        {
            if (enigma::voxel::block::AIR)
            {
                int32_t blockX = static_cast<int32_t>(std::floor(m_position.x));
                int32_t blockY = static_cast<int32_t>(std::floor(m_position.y));
                int32_t blockZ = static_cast<int32_t>(std::floor(m_position.z));

                auto topBlockZ       = m_game->m_world->GetTopBlockZ(enigma::voxel::block::BlockPos(blockX, blockY, blockZ));
                auto blockDestroyPos = enigma::voxel::block::BlockPos(blockX, blockY, topBlockZ);
                m_game->m_world->SetBlockState(blockDestroyPos, enigma::voxel::block::AIR->GetDefaultState());
            }
        }
    }
}

void Player::HandleCameraModeSwitch()
{
    if (g_theInput->WasKeyJustPressed('C'))
    {
        switch (m_cameraMode)
        {
        case CameraMode::SPECTATOR_FULL:
            m_cameraMode = CameraMode::SPECTATOR_XY;
            break;
        case CameraMode::SPECTATOR_XY:
            m_cameraMode = CameraMode::THIRD_PERSON;
            break;
        case CameraMode::THIRD_PERSON:
            m_cameraMode = CameraMode::FIRST_PERSON;
            break;
        case CameraMode::FIRST_PERSON:
            m_cameraMode = CameraMode::SPECTATOR_FULL;
            break;
        }
    }
}

void Player::HandleMouseAndControllerInput(float deltaSeconds)
{
    // Mouse input for camera orientation
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
    m_orientation.m_yawDegrees += -cursorDelta.x * 0.125f;
    m_orientation.m_pitchDegrees += -cursorDelta.y * 0.125f;

    // Controller input for camera orientation
    const XboxController& controller    = g_theInput->GetController(0);
    Vec2                  rightStickPos = controller.GetRightStick().GetPosition();
    float                 rightStickMag = controller.GetRightStick().GetMagnitude();
    float                 speed         = 4.0f;

    if (rightStickMag > 0.f)
    {
        m_orientation.m_yawDegrees += -(rightStickPos * speed * rightStickMag * 0.125f).x;
        m_orientation.m_pitchDegrees += -(rightStickPos * speed * rightStickMag * 0.125f).y;
    }

    // Controller trigger input for roll
    float leftTrigger  = controller.GetLeftTrigger();
    float rightTrigger = controller.GetRightTrigger();
    m_orientation.m_rollDegrees += leftTrigger * 0.125f * deltaSeconds * speed;
    m_orientation.m_rollDegrees -= rightTrigger * 0.125f * deltaSeconds * speed;

    // Clamp orientation values
    m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
    m_orientation.m_rollDegrees  = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
}

void Player::HandleMovementInput(float deltaSeconds)
{
    const XboxController& controller = g_theInput->GetController(0);
    float                 speed      = 4.0f;

    // Sprint speed modifier
    if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A))
    {
        speed *= 20.f;
    }

    // Get movement vectors
    Vec3 forward, left, up;
    m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

    // Movement input processing
    Vec3 movementDelta = Vec3::ZERO;

    // Controller movement
    Vec2  leftStickPos = controller.GetLeftStick().GetPosition();
    float leftStickMag = controller.GetLeftStick().GetMagnitude();
    movementDelta += (leftStickPos * speed * leftStickMag * deltaSeconds).y * forward;
    movementDelta += -(leftStickPos * speed * leftStickMag * deltaSeconds).x * left;

    // Keyboard movement
    if (g_theInput->IsKeyDown('W'))
        movementDelta += forward * speed * deltaSeconds;
    if (g_theInput->IsKeyDown('S'))
        movementDelta -= forward * speed * deltaSeconds;
    if (g_theInput->IsKeyDown('A'))
        movementDelta += left * speed * deltaSeconds;
    if (g_theInput->IsKeyDown('D'))
        movementDelta -= left * speed * deltaSeconds;

    // Vertical movement - camera mode dependent
    if (m_cameraMode == CameraMode::SPECTATOR_FULL)
    {
        if (g_theInput->IsKeyDown('Q') || controller.IsButtonDown(XBOX_BUTTON_RS))
            movementDelta.z -= deltaSeconds * speed;
        if (g_theInput->IsKeyDown('E') || controller.IsButtonDown(XBOX_BUTTON_LS))
            movementDelta.z += deltaSeconds * speed;
    }
    else if (m_cameraMode == CameraMode::SPECTATOR_XY)
    {
        // In SpectatorXY mode, restrict movement to XY plane only
        movementDelta.z = 0.0f;
    }

    // Apply movement
    m_position += movementDelta;
}

void Player::UpdateCameraSettings()
{
    m_camera->SetPerspectiveView(g_theWindow->GetClientAspectRatio(), 60.f, 0.1f, 2048.f);
    m_camera->SetPosition(m_position);
    m_camera->SetOrientation(m_orientation);

    // Set camera transform matrix
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_camera->SetCameraToRenderTransform(ndcMatrix);
}
