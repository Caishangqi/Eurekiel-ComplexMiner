#include "GUIProfiler.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Game.hpp"

bool GUIProfiler::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)

    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUIProfiler)));
    if (gui) g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUIProfiler::GUIProfiler() : GUI()
{
    // Event
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
    m_vertices.reserve(2048);
}

void GUIProfiler::Draw()
{
    g_theRenderer->BindTexture(m_fontTexture);
    g_theRenderer->DrawVertexArray(m_vertices);
}

void GUIProfiler::DrawHud()
{
}

void GUIProfiler::Update(float deltaTime)
{
    UNUSED(deltaTime)

    if (m_threadPoolUpdateTimer.HasPeriodElapsed())
    {
        m_numOfPendingTaskChunkGen   = g_theSchedule->GetPendingTaskCount("ChunkGen");
        m_numOfExecutingTaskChunkGen = g_theSchedule->GetExecutingTaskCount("ChunkGen");
        m_numOfCompleteTaskChunkGen  = g_theSchedule->GetCompletedTaskCount("ChunkGen");

        m_numOfPendingTaskMeshBuilding   = g_theSchedule->GetPendingTaskCount("MeshBuilding");
        m_numOfExecutingTaskMeshBuilding = g_theSchedule->GetExecutingTaskCount("MeshBuilding");
        m_numOfCompleteTaskMeshBuilding  = g_theSchedule->GetCompletedTaskCount("MeshBuilding");

        m_numOfPendingTaskFileIO   = g_theSchedule->GetPendingTaskCount("FileIO");
        m_numOfExecutingTaskFileIO = g_theSchedule->GetExecutingTaskCount("FileIO");
        m_numOfCompleteTaskFileIO  = g_theSchedule->GetCompletedTaskCount("FileIO");
        m_threadPoolUpdateTimer.Start();
    }

    m_numOpaqueVertices      = 0;
    m_numTransparentVertices = 0;

    m_numOpaqueIndices      = 0;
    m_numTransparentIndices = 0;

    m_numOpaqueTriangles      = 0;
    m_numTransparentTriangles = 0;

    // Fetch information from chunk
    const auto& loadedChunks = g_theGame->m_world->GetChunkManager()->GetLoadedChunks();
    m_numChunkLoaded         = (int)loadedChunks.size();

    for (auto& pair : loadedChunks)
    {
        auto mesh = pair.second->GetMesh();
        if (mesh)
        {
            m_numOpaqueVertices += mesh->GetOpaqueVertexCount();
            m_numTransparentVertices += mesh->GetTransparentVertexCount();
            m_numOpaqueIndices += mesh->GetOpaqueIndexCount();
            m_numTransparentIndices += mesh->GetTransparentIndexCount();
            m_numOpaqueTriangles += mesh->GetOpaqueTriangleCount();
            m_numTransparentTriangles += mesh->GetTransparentTriangleCount();
        }
    }

    m_vertices.clear();
    // Chunks Pool Statistic:
    // 4 (chunks Loaded)
    AABB2 poolStatistPanel = m_config.screenSpace.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, "Chunks Pool Statistic:", poolStatistPanel, 12.f, Rgba8::WHITE, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelNumLoaded = poolStatistPanel.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (chunks Loaded)", m_numChunkLoaded), poolStatistPanelNumLoaded, 12.f, Rgba8::ORANGE, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelOpaqueVertices = poolStatistPanelNumLoaded.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (opaque vertices)", m_numOpaqueVertices), poolStatistPanelOpaqueVertices, 12.f, Rgba8::DEBUG_GREEN, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelTransparentVertices = poolStatistPanelOpaqueVertices.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (transparent vertices)", m_numTransparentVertices), poolStatistPanelTransparentVertices, 12.f, Rgba8::DEBUG_GREEN, 1,
                                             Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelOpaqueIndices = poolStatistPanelTransparentVertices.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (opaque indices)", m_numOpaqueIndices), poolStatistPanelOpaqueIndices, 12.f, Rgba8::YELLOW, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelTransparentIndices = poolStatistPanelOpaqueIndices.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (transparent indices)", m_numTransparentIndices), poolStatistPanelTransparentIndices, 12.f, Rgba8::YELLOW, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelOpaqueTriangles = poolStatistPanelTransparentIndices.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (opaque triangles)", m_numOpaqueTriangles), poolStatistPanelOpaqueTriangles, 12.f, Rgba8::ORANGE, 1, Vec2(0.0f, 1.0f));
    AABB2 poolStatistPanelTransparentTriangles = poolStatistPanelOpaqueTriangles.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%d (opaque triangles)", m_numTransparentTriangles), poolStatistPanelTransparentTriangles, 12.f, Rgba8::ORANGE, 1, Vec2(0.0f, 1.0f));

    // Thread Pool Statistic:
    AABB2 threadPoolStatistPanel = m_config.screenSpace.GetPadded(Vec4(0, 0, 0, -256));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, "Task Schedule Statistic:", threadPoolStatistPanel, 12.f, Rgba8::WHITE, 1, Vec2(0.0f, 1.0f));
    AABB2 TaskPanelChunkGen = threadPoolStatistPanel.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(
        m_vertices, Stringf(
            "ChunkGen:      (Pending: %d | Executing: %d | Complete: %d)"
            , m_numOfPendingTaskChunkGen, m_numOfExecutingTaskChunkGen, m_numOfCompleteTaskChunkGen), TaskPanelChunkGen, 12.f, Rgba8::DEBUG_GREEN, 1, Vec2(0.0f, 1.0f));
    AABB2 TaskPanelMeshBuilding = TaskPanelChunkGen.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(
        m_vertices, Stringf(
            "MeshBuilding:  (Pending: %d | Executing: %d | Complete: %d)"
            , m_numOfPendingTaskMeshBuilding, m_numOfExecutingTaskMeshBuilding, m_numOfCompleteTaskMeshBuilding), TaskPanelMeshBuilding, 12.f, Rgba8::DEBUG_GREEN, 1, Vec2(0.0f, 1.0f));
    AABB2 TaskPanelMeshFileIO = TaskPanelMeshBuilding.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(
        m_vertices, Stringf(
            "FileIO:        (Pending: %d | Executing: %d | Complete: %d)"
            , m_numOfPendingTaskFileIO, m_numOfExecutingTaskFileIO, m_numOfCompleteTaskFileIO), TaskPanelMeshFileIO, 12.f, Rgba8::DEBUG_GREEN, 1, Vec2(0.0f, 1.0f));
}

void GUIProfiler::OnCreate()
{
    m_fontTexture           = &m_defaultGUIFont->GetTexture();
    m_threadPoolUpdateTimer = Timer(0.05f, g_theGame->m_clock);
    m_threadPoolUpdateTimer.Start();
}

void GUIProfiler::OnDestroy()
{
    m_threadPoolUpdateTimer.Stop();
}
