#include "GUIGrid.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"

GUIGrid::GUIGrid() : GUI()
{
    g_theRenderer->CreateOrGetBitmapFont("Run/.enigma/data/Fonts/SquirrelFixedFont");
}

GUIGrid::~GUIGrid()
{
}

void GUIGrid::Draw()
{
}

void GUIGrid::Update(float deltaTime)
{
    UNUSED(deltaTime)
}

void GUIGrid::OnCreate()
{
}

void GUIGrid::OnDestroy()
{
}
