#include "GUIPlayerInventory.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"


bool GUIPlayerInventory::Event_Player_Quit_World(EventArgs& args)
{
    UNUSED(args)
    auto gui = g_theGUI->GetGUI(std::type_index(typeid(GUIPlayerInventory)));
    if (gui) g_theGUI->RemoveFromViewPort(gui);
    return false;
}

GUIPlayerInventory::GUIPlayerInventory()
{
    // Event
    g_theEventSystem->SubscribeEventCallbackFunction("Event.PlayerQuitWorld", Event_Player_Quit_World);
    m_vertices.reserve(2048);

    m_blocks    = enigma::registry::block::BlockRegistry::GetAllBlocks();
    m_numBlocks = (unsigned int)m_blocks.size();
}

GUIPlayerInventory::~GUIPlayerInventory()
{
}

void GUIPlayerInventory::Draw()
{
    g_theRenderer->BindTexture(&m_defaultGUIFont->GetTexture());
    g_theRenderer->DrawVertexArray(m_vertices);
}

void GUIPlayerInventory::DrawHud()
{
}

void GUIPlayerInventory::Update(float deltaTime)
{
    UNUSED(deltaTime)
    ProcessInput(deltaTime);
    m_vertices.clear();

    AABB2 hudNamePanel = m_config.screenSpace.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, "Debug Block Selection", hudNamePanel, 12.f, Rgba8::WHITE, 1, Vec2(0.5f, 1.0f));

    AABB2 preBlock = hudNamePanel.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%s", GetPreBlock()->GetRegistryName().c_str()), preBlock, 10.f, Rgba8(255, 160, 0, 125), 1, Vec2(0.5f, 1.0f));
    AABB2 currentBlock = preBlock.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%s", GetCurrentBlock()->GetRegistryName().c_str()), currentBlock, 12.f, Rgba8::ORANGE, 1, Vec2(0.5f, 1.0f));
    AABB2 nextBlock = currentBlock.GetPadded(Vec4(0, 0, 0, -16));
    m_defaultGUIFont->AddVertsForTextInBox2D(m_vertices, Stringf("%s", GetNextBlock()->GetRegistryName().c_str()), nextBlock, 10.f, Rgba8(255, 160, 0, 125), 1, Vec2(0.5f, 1.0f));
}

void GUIPlayerInventory::OnCreate()
{
}

void GUIPlayerInventory::OnDestroy()
{
}

std::shared_ptr<enigma::registry::block::Block> GUIPlayerInventory::GetPreBlock()
{
    if (m_currentIndex == 0)
    {
        return m_blocks[m_numBlocks - 1];
    }
    return m_blocks[m_currentIndex - 1];
}

std::shared_ptr<enigma::registry::block::Block> GUIPlayerInventory::GetCurrentBlock()
{
    return m_blocks[m_currentIndex];
}

std::shared_ptr<enigma::registry::block::Block> GUIPlayerInventory::GetNextBlock()
{
    if (m_currentIndex == m_numBlocks - 1)
    {
        return m_blocks[0];
    }
    return m_blocks[m_currentIndex + 1];
}

void GUIPlayerInventory::ProcessInput(float deltaTime)
{
    UNUSED(deltaTime)
    short delta = g_theInput->GetMouseWheelDelta();
    if (delta > 0)
    {
        m_currentIndex = (m_currentIndex + 1) % m_numBlocks;
    }
    else if (delta < 0)
    {
        m_currentIndex = (m_currentIndex + m_numBlocks - 1) % m_numBlocks;
    }
}
