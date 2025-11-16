#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Registry/Block/Block.hpp"
#include "Engine/Voxel/Block/BlockPos.hpp"
#include "Game/Framework/GUISubsystem.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Voxel/World/VoxelRaycastResult3D.hpp"

struct Vertex_PCU;
class Player;

class GUIBlock3DSelection : public GUI
{
    friend class Player;

    DECLARE_GUI(GUIBlock3DSelection, "GUIBlock3DSelection", 80)

public:
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    GUIBlock3DSelection();
    ~GUIBlock3DSelection() override;

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

    // [NEW] Public accessor method - used for Player interaction logic
    const enigma::voxel::VoxelRaycastResult3D& GetCurrentRaycast() const { return m_currentRaycast; }
    bool                                       IsRaycastLocked() const { return m_isRaycastLocked; }

private:
    // [NEW] Member variables related to the R key lock mechanism
    bool m_isRaycastLocked = false; // Whether the ray is locked
    Vec3 m_lockedCameraPos; // Camera position when locked
    Vec3 m_lockedCameraForward; // Camera front vector when locked

    // [NEW] Ray detection results
    enigma::voxel::VoxelRaycastResult3D m_currentRaycast; // Current ray detection result

    // [OLD] 原有成员变量
    Player*                 m_player = nullptr;
    std::vector<Vertex_PCU> m_vertices;
    enigma::voxel::BlockPos m_blockPosition;
    AABB3                   m_unitBlock = AABB3(Vec3(0, 0, 0), Vec3(1, 1, 1));

    // [NEW] Auxiliary method: Calculate the 4 corner points of the surface
    void CalculateFaceQuadCorners(const enigma::voxel::BlockPos& blockPos, enigma::voxel::Direction face, Vec3 outCorners[4]) const;
    Vec3 GetNormalFromFace(enigma::voxel::Direction face) const;
};
