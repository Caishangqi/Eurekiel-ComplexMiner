#include "GUIBlock3DSelection.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Game/Gameplay/Player/GameCamera.hpp"
#include "Game/Gameplay/Player/CameraMode.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Registry/Block/Block.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Engine/Voxel/Block/BlockIterator.hpp"
#include "Engine/Voxel/Block/BlockState.hpp"
#include "Engine/Voxel/Block/VoxelShape.hpp"
#include "Engine/Voxel/World/VoxelRaycastResult3D.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/Player/Player.hpp"

using enigma::registry::block::Block;
using enigma::voxel::BlockState;
using enigma::voxel::VoxelShape;

bool GUIBlock3DSelection::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)
    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUIBlock3DSelection)));
    if (gui) g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUIBlock3DSelection::GUIBlock3DSelection()
{
    m_vertices.reserve(1024);
}

GUIBlock3DSelection::~GUIBlock3DSelection()
{
}

void GUIBlock3DSelection::Draw()
{
}

void GUIBlock3DSelection::DrawHud()
{
    std::vector<Vertex_PCU> tempVerts;
    tempVerts.reserve(256);

    // [STEP 1] If locked, draw 3D ray
    if (m_isRaycastLocked)
    {
        Vec3 rayEnd = m_lockedCameraPos + m_lockedCameraForward * 16.0f;

        //Draw arrow (yellow indicates locked state)
        Rgba8 rayColor = m_currentRaycast.m_didImpact ? Rgba8::YELLOW : Rgba8::ORANGE;
        AddVertsForArrow3DFixArrowSize(tempVerts, rayEnd, m_lockedCameraPos, 0.02f, 0.15f, rayColor);
    }

    // [STEP 2] If the block is hit, draw the highlight
    if (m_currentRaycast.m_didImpact)
    {
        using namespace enigma::voxel;
        BlockIterator hitIter = m_currentRaycast.m_hitBlockIter;
        BlockPos      hitPos  = hitIter.GetBlockPos();

        // [2.1] Draw the outer frame of the box (white wireframe)
        Vec3 cubeMin(static_cast<float>(hitPos.x),
                     static_cast<float>(hitPos.y),
                     static_cast<float>(hitPos.z));
        Vec3 cubeMax = cubeMin + Vec3(1.0f, 1.0f, 1.0f);

        AABB3 blockBounds(cubeMin, cubeMax);
        AddVertsForCube3DWireFrame(tempVerts, blockBounds, Rgba8::WHITE, 0.02f);

        // [2.2] Draw hit surface highlight (green translucent)
        Vec3 quadCorners[4];
        CalculateFaceQuadCorners(hitPos, m_currentRaycast.m_hitFace, quadCorners);

        Rgba8 faceColor(0, 255, 0, 100); // 绿色半透明
        AddVertsForQuad3D(tempVerts, quadCorners[0], quadCorners[1],
                          quadCorners[2], quadCorners[3], faceColor);

        // [2.3] Draw VoxelShape collision boxes (orange) for non-full blocks
        BlockState* hitState = hitIter.GetBlock();
        if (hitState && !hitState->IsFullOpaque())
        {
            Block* hitBlock = hitState->GetBlock();
            if (hitBlock)
            {
                VoxelShape collisionShape = hitBlock->GetCollisionShape(hitState);
                if (!collisionShape.IsEmpty())
                {
                    Vec3 blockWorldPos(static_cast<float>(hitPos.x),
                                       static_cast<float>(hitPos.y),
                                       static_cast<float>(hitPos.z));

                    Vec3            impactPos    = m_currentRaycast.m_impactPos;
                    Vec3            impactNormal = m_currentRaycast.m_impactNormal;
                    Rgba8           shapeWireColor(255, 165, 0, 255); // Orange
                    Rgba8           shapeFaceColor(255, 165, 0, 80); // Orange translucent
                    constexpr float EPSILON = 0.01f; // Tolerance for face detection

                    // Draw each collision box in the shape
                    const auto& boxes = collisionShape.GetBoxes();
                    for (const auto& localBox : boxes)
                    {
                        // Transform local box to world coordinates
                        AABB3 worldBox(
                            blockWorldPos + localBox.m_mins,
                            blockWorldPos + localBox.m_maxs
                        );

                        // Draw orange wireframe for collision shape
                        AddVertsForCube3DWireFrame(tempVerts, worldBox, shapeWireColor, 0.015f);

                        // Only draw hit face for the box that contains the impact point
                        // Check if impact point is on one of this box's faces
                        bool isHitBox = false;

                        // Check +Z face (top)
                        if (impactNormal.z > 0.5f &&
                            std::abs(impactPos.z - worldBox.m_maxs.z) < EPSILON &&
                            impactPos.x >= worldBox.m_mins.x - EPSILON && impactPos.x <= worldBox.m_maxs.x + EPSILON &&
                            impactPos.y >= worldBox.m_mins.y - EPSILON && impactPos.y <= worldBox.m_maxs.y + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              shapeFaceColor);
                        }
                        // Check -Z face (bottom)
                        else if (impactNormal.z < -0.5f &&
                            std::abs(impactPos.z - worldBox.m_mins.z) < EPSILON &&
                            impactPos.x >= worldBox.m_mins.x - EPSILON && impactPos.x <= worldBox.m_maxs.x + EPSILON &&
                            impactPos.y >= worldBox.m_mins.y - EPSILON && impactPos.y <= worldBox.m_maxs.y + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              shapeFaceColor);
                        }
                        // Check +X face (East)
                        else if (impactNormal.x > 0.5f &&
                            std::abs(impactPos.x - worldBox.m_maxs.x) < EPSILON &&
                            impactPos.y >= worldBox.m_mins.y - EPSILON && impactPos.y <= worldBox.m_maxs.y + EPSILON &&
                            impactPos.z >= worldBox.m_mins.z - EPSILON && impactPos.z <= worldBox.m_maxs.z + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              shapeFaceColor);
                        }
                        // Check -X face (West)
                        else if (impactNormal.x < -0.5f &&
                            std::abs(impactPos.x - worldBox.m_mins.x) < EPSILON &&
                            impactPos.y >= worldBox.m_mins.y - EPSILON && impactPos.y <= worldBox.m_maxs.y + EPSILON &&
                            impactPos.z >= worldBox.m_mins.z - EPSILON && impactPos.z <= worldBox.m_maxs.z + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              shapeFaceColor);
                        }
                        // Check +Y face (North)
                        else if (impactNormal.y > 0.5f &&
                            std::abs(impactPos.y - worldBox.m_maxs.y) < EPSILON &&
                            impactPos.x >= worldBox.m_mins.x - EPSILON && impactPos.x <= worldBox.m_maxs.x + EPSILON &&
                            impactPos.z >= worldBox.m_mins.z - EPSILON && impactPos.z <= worldBox.m_maxs.z + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_maxs.y, worldBox.m_mins.z),
                                              shapeFaceColor);
                        }
                        // Check -Y face (South)
                        else if (impactNormal.y < -0.5f &&
                            std::abs(impactPos.y - worldBox.m_mins.y) < EPSILON &&
                            impactPos.x >= worldBox.m_mins.x - EPSILON && impactPos.x <= worldBox.m_maxs.x + EPSILON &&
                            impactPos.z >= worldBox.m_mins.z - EPSILON && impactPos.z <= worldBox.m_maxs.z + EPSILON)
                        {
                            isHitBox = true;
                            AddVertsForQuad3D(tempVerts,
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              Vec3(worldBox.m_mins.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_maxs.z),
                                              Vec3(worldBox.m_maxs.x, worldBox.m_mins.y, worldBox.m_mins.z),
                                              shapeFaceColor);
                        }

                        UNUSED(isHitBox);
                    }
                }
            }
        }

        // [NEW] Draw the hit point (red ball)
        Vec3  impactPos     = m_currentRaycast.m_impactPos;
        float sphereRadius  = 0.05f; // radius of the ball
        int   sphereSlices  = 8; // Longitude direction segmentation (low number of polygons)
        int   sphereStacks  = 6; // Latitude segmentation (low number of polygons)
        Rgba8 hitPointColor = Rgba8::RED; // red hit point

        AddVertsForSphere3D(tempVerts, impactPos, sphereRadius, hitPointColor, AABB2::ZERO_TO_ONE, sphereSlices, sphereStacks);

        // [NEW] Draw the normal of the hit surface (cyan arrow)
        Vec3  normalStart = impactPos;
        Vec3  normalEnd   = impactPos + m_currentRaycast.m_impactNormal * 1.0f; // The length is 1 meter
        float arrowRadius = 0.03f; // Arrow stem radius
        float arrowSize   = 0.15f; // Arrow head size
        Rgba8 normalColor = Rgba8::CYAN; // Cyan normal

        AddVertsForArrow3DFixArrowSize(tempVerts, normalEnd, normalStart, arrowRadius, arrowSize, normalColor);
    }

    // [STEP 3] 绘制所有顶点
    if (!tempVerts.empty())
    {
        g_theRenderer->SetDepthMode(depth_mode::DISABLED);
        g_theRenderer->SetModelConstants();
        g_theRenderer->SetBlendMode(blend_mode::ALPHA);
        g_theRenderer->BindTexture(nullptr);
        g_theRenderer->DrawVertexArray(tempVerts);
        g_theRenderer->SetBlendMode(blend_mode::OPAQUE);
    }
}

void GUIBlock3DSelection::Update(float deltaTime)
{
    UNUSED(deltaTime)

    // [STEP 1] R key switches lock state
    // [IMPORTANT] R key is disabled in SPECTATOR and SPECTATOR_XY modes (per Task 3.4 requirement)
    CameraMode cameraMode      = m_player->GetCamera()->GetCameraMode();
    bool       isSpectatorMode = (cameraMode == CameraMode::SPECTATOR || cameraMode == CameraMode::SPECTATOR_XY);

    if (g_theInput->WasKeyJustPressed('R') && !isSpectatorMode)
    {
        m_isRaycastLocked = !m_isRaycastLocked;

        if (m_isRaycastLocked)
        {
            // [GOOD] Lock: record the current player eye position and aim
            m_lockedCameraPos     = m_player->m_position + m_player->m_eyeOffset;
            m_lockedCameraForward = m_player->m_aim.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
        }
        // No need to clear when unlocking, it will be overwritten the next time you lock
    }

    // [STEP 2] Perform ray detection
    Vec3 rayStart;
    Vec3 rayDir;

    if (m_isRaycastLocked)
    {
        // [LOCKED] Use locked position and direction
        rayStart = m_lockedCameraPos;
        rayDir   = m_lockedCameraForward;
    }
    else if (isSpectatorMode)
    {
        // [SPECTATOR/SPECTATOR_XY] Raycast based on camera position and orientation
        rayStart = m_player->GetCamera()->GetPosition();
        rayDir   = m_player->GetCamera()->GetOrientation().GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
    }
    else
    {
        // [NORMAL] Raycast based on player eye position and aim
        rayStart = m_player->m_position + m_player->m_eyeOffset;
        rayDir   = m_player->m_aim.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
    }

    // Call World::RaycastVsBlocks (maximum distance 16 meters)
    m_currentRaycast = g_theGame->m_world->RaycastVsBlocks(rayStart, rayDir, 16.0f);

    m_hudCamera->SetPosition(m_player->GetCamera()->GetPosition());
    m_hudCamera->SetOrientation(m_player->GetCamera()->GetOrientation());
}

void GUIBlock3DSelection::OnCreate()
{
    m_player = g_theGame->m_player;
    AddVertsForCube3DWireFrame(m_vertices, m_unitBlock, Rgba8::WHITE);
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
}

void GUIBlock3DSelection::OnDestroy()
{
}

//--------------------------------------------------------------------------------------------------------------------------------
// [NEW] Calculate the 4 corner points of the surface (counterclockwise order)
//--------------------------------------------------------------------------------------------------------------------------------
void GUIBlock3DSelection::CalculateFaceQuadCorners(const enigma::voxel::BlockPos& blockPos, enigma::voxel::Direction face, Vec3 outCorners[4]) const
{
    using namespace enigma::voxel;
#undef min
    Vec3 min(static_cast<float>(blockPos.x),
             static_cast<float>(blockPos.y),
             static_cast<float>(blockPos.z));
    Vec3 max = min + Vec3(1.0f, 1.0f, 1.0f);

    // Calculate 4 corner points according to the face direction (counterclockwise order, used in AddVertsForQuad3D)
    switch (face)
    {
    case Direction::EAST: // +X side
        outCorners[0] = Vec3(max.x, min.y, min.z);
        outCorners[1] = Vec3(max.x, max.y, min.z);
        outCorners[2] = Vec3(max.x, max.y, max.z);
        outCorners[3] = Vec3(max.x, min.y, max.z);
        break;

    case Direction::WEST: // -X side
        outCorners[0] = Vec3(min.x, min.y, min.z);
        outCorners[1] = Vec3(min.x, min.y, max.z);
        outCorners[2] = Vec3(min.x, max.y, max.z);
        outCorners[3] = Vec3(min.x, max.y, min.z);
        break;

    case Direction::NORTH: // +Y side
        outCorners[0] = Vec3(min.x, max.y, min.z);
        outCorners[1] = Vec3(min.x, max.y, max.z);
        outCorners[2] = Vec3(max.x, max.y, max.z);
        outCorners[3] = Vec3(max.x, max.y, min.z);
        break;

    case Direction::SOUTH: // -Y side
        outCorners[0] = Vec3(min.x, min.y, min.z);
        outCorners[1] = Vec3(max.x, min.y, min.z);
        outCorners[2] = Vec3(max.x, min.y, max.z);
        outCorners[3] = Vec3(min.x, min.y, max.z);
        break;

    case Direction::UP: // +Z plane
        outCorners[0] = Vec3(min.x, min.y, max.z);
        outCorners[1] = Vec3(max.x, min.y, max.z);
        outCorners[2] = Vec3(max.x, max.y, max.z);
        outCorners[3] = Vec3(min.x, max.y, max.z);
        break;

    case Direction::DOWN: // -Z plane
        outCorners[0] = Vec3(min.x, min.y, min.z);
        outCorners[1] = Vec3(min.x, max.y, min.z);
        outCorners[2] = Vec3(max.x, max.y, min.z);
        outCorners[3] = Vec3(max.x, min.y, min.z);
        break;

    default:
        // Unknown direction, use default value
        outCorners[0] = min;
        outCorners[1] = min;
        outCorners[2] = min;
        outCorners[3] = min;
        break;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------
// [NEW] Get the normal vector of the surface
//--------------------------------------------------------------------------------------------------------------------------------
Vec3 GUIBlock3DSelection::GetNormalFromFace(enigma::voxel::Direction face) const
{
    using namespace enigma::voxel;

    switch (face)
    {
    case Direction::EAST: return Vec3(1.0f, 0.0f, 0.0f);
    case Direction::WEST: return Vec3(-1.0f, 0.0f, 0.0f);
    case Direction::NORTH: return Vec3(0.0f, 1.0f, 0.0f);
    case Direction::SOUTH: return Vec3(0.0f, -1.0f, 0.0f);
    case Direction::UP: return Vec3(0.0f, 0.0f, 1.0f);
    case Direction::DOWN: return Vec3(0.0f, 0.0f, -1.0f);
    default: return Vec3(0.0f, 0.0f, 0.0f);
    }
}
