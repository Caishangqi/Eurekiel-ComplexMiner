#include "GUIDebugLight.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Voxel/World/World.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Gameplay/entity/Player.hpp"

GUIDebugLight::GUIDebugLight()
{
    m_blocks.reserve(65536);
    m_player = g_theGame->m_player;
    m_world  = g_theGame->m_world.get();
    m_timer  = new Timer(0.2f);
    m_timer->Start();
}

GUIDebugLight::~GUIDebugLight()
{
}

void GUIDebugLight::Draw()
{
}

void GUIDebugLight::DrawHud()
{
}

void GUIDebugLight::Update(float deltaTime)
{
    if (m_timer->GetElapsedFraction() > 0.9f)
    {
    }
    PopulateDebugBlocks();
    // [NEW] 处理 m_blocks：为每个方块显示光照信息
    for (const DebugBlockInfo& info : m_blocks)
    {
        if (info.state == nullptr) continue;

        // [NEW] 使用 World 的全局坐标接口获取光照值
        uint8_t outdoorLight = m_world->GetOutdoorLight(info.pos.x, info.pos.y, info.pos.z); // 天空光 (0-15)
        uint8_t indoorLight  = m_world->GetIndoorLight(info.pos.x, info.pos.y, info.pos.z); // 方块光 (0-15)

        // [NEW] 计算方块中心位置（用于文本显示）
        Vec3 blockCenter(
            static_cast<float>(info.pos.x) + 0.5f,
            static_cast<float>(info.pos.y) + 0.5f,
            static_cast<float>(info.pos.z) + 0.5f
        );

        // [NEW] 使用 Billboard 文本在世界空间显示光照值
        // 参数说明：
        // - lightText: 显示的文本内容
        // - blockCenter: 方块中心位置
        // - 0.15f: 文本高度（较小以避免遮挡）
        // - Rgba8::YELLOW: 黄色文本（醒目）
        // - Rgba8::YELLOW: 结束颜色（不渐变）
        // - DebugRenderMode::USE_DEPTH: 使用深度测试（被遮挡时不显示）
        // - Vec2(0.5f, 0.5f): 居中对齐
        // - 0.0f: 持续时间（0表示仅显示一帧）
        DebugAddWorldBillboardText(
            Stringf("O:%d I:%d", outdoorLight, indoorLight),
            blockCenter,
            0.15f,
            Rgba8::YELLOW,
            Rgba8::YELLOW,
            DebugRenderMode::USE_DEPTH,
            Vec2(0.5f, 0.5f),
            0.0f
        );
    }
}

void GUIDebugLight::OnCreate()
{
}

void GUIDebugLight::OnDestroy()
{
}

void GUIDebugLight::PopulateDebugBlocks()
{
    if (!m_world || !m_player) return;
    m_blocks.clear();

    // [NEW] 获取玩家位置并计算遍历范围
    Vec3    playerPos = m_player->m_position;
    int32_t centerX   = static_cast<int32_t>(playerPos.x);
    int32_t centerY   = static_cast<int32_t>(playerPos.y);
    int32_t centerZ   = static_cast<int32_t>(playerPos.z);

    // [NEW] 计算边界：玩家位置 ± m_debugRadius
    int32_t minX = centerX - m_debugRadius;
    int32_t maxX = centerX + m_debugRadius;
    int32_t minY = centerY - m_debugRadius;
    int32_t maxY = centerY + m_debugRadius;
    int32_t minZ = centerZ - m_debugRadius;
    int32_t maxZ = centerZ + m_debugRadius;

    // [NEW] 遍历范围内的所有方块
    for (int32_t x = minX; x <= maxX; ++x)
    {
        for (int32_t y = minY; y <= maxY; ++y)
        {
            for (int32_t z = minZ; z <= maxZ; ++z)
            {
                enigma::voxel::BlockPos    pos(x, y, z);
                enigma::voxel::BlockState* state = m_world->GetBlockState(pos);

                // [NEW] 过滤条件：跳过 null 和不可见方块（如空气）
                if (state == nullptr) continue;
                //if (!state->IsVisible()) continue;        // 跳过不可见方块（如空气）
                // [FIX] 移除 IsFullOpaque() 过滤，显示所有可见方块的光照值（包括不透明方块）

                // [NEW] 添加到调试列表（同时存储状态和位置）
                DebugBlockInfo info;
                info.state = state;
                info.pos   = pos;
                m_blocks.push_back(info);
            }
        }
    }
}
