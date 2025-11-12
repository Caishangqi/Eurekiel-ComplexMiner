#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Timer.hpp"
#include "Game/Framework/GUISubsystem.hpp"

struct Vertex_PCU;
class Texture;

class GUIProfiler : public GUI
{
    DECLARE_GUI(GUIProfiler, "GUIProfiler", 100)

public:
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    explicit GUIProfiler();

    void Draw() override;
    void DrawHud() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    Texture*                m_fontTexture = nullptr;
    std::vector<Vertex_PCU> m_vertices;

    size_t m_numChunkLoaded          = 0;
    size_t m_numOpaqueVertices       = 0;
    size_t m_numTransparentVertices  = 0;
    size_t m_numOpaqueIndices        = 0;
    size_t m_numTransparentIndices   = 0;
    size_t m_numOpaqueTriangles      = 0;
    size_t m_numTransparentTriangles = 0;

    int32_t m_numOfPendingTaskChunkGen   = 0;
    int32_t m_numOfExecutingTaskChunkGen = 0;
    int32_t m_numOfCompleteTaskChunkGen  = 0;

    int32_t m_numOfPendingTaskMeshBuilding   = 0;
    int32_t m_numOfExecutingTaskMeshBuilding = 0;
    int32_t m_numOfCompleteTaskMeshBuilding  = 0;

    int32_t m_numOfPendingTaskFileIO   = 0;
    int32_t m_numOfExecutingTaskFileIO = 0;
    int32_t m_numOfCompleteTaskFileIO  = 0;

private:
    Timer                  m_threadPoolUpdateTimer;
    Timer                  m_vertexCountUpdateTimer; // Timer to throttle expensive vertex statistics updates
    static constexpr float VERTEX_UPDATE_INTERVAL = 0.5f; // Update every 0.5 seconds (2 Hz)
};
