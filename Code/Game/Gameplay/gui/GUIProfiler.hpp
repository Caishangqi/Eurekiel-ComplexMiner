#pragma once
#include "Engine/Core/EventSystem.hpp"
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
};
