#pragma once
#include "../Block/BlockDefinition.hpp"

namespace simpleminer::framework::world
{
    using namespace simpleminer::framework::block;

    /**
     * Block - 轻量级实例，遵循Flyweight模式
     * 类似于Minecraft的BlockState概念，但更简化
     * 存储对BlockDefinition的引用和可能的状态数据
     */
    struct Block
    {
    public:
        // 构造函数
        Block() : m_definition(nullptr), m_stateData(0)
        {
        }

        explicit Block(const BlockDefinition* definition) : m_definition(definition), m_stateData(0)
        {
        }

        Block(const BlockDefinition* definition, uint8_t stateData) : m_definition(definition), m_stateData(stateData)
        {
        }

        // 基本属性访问（委托给BlockDefinition）
        bool  IsVisible() const { return m_definition ? m_definition->IsVisible() : false; }
        bool  IsSolid() const { return m_definition ? m_definition->IsSolid() : false; }
        bool  IsOpaque() const { return m_definition ? m_definition->IsOpaque() : false; }
        int   GetLightEmission() const { return m_definition ? m_definition->GetLightEmission() : 0; }
        float GetHardness() const { return m_definition ? m_definition->GetHardness() : 0.0f; }

        // BlockDefinition访问
        const BlockDefinition* GetDefinition() const { return m_definition; }
        void                   SetDefinition(const BlockDefinition* definition) { m_definition = definition; }

        // 状态数据（用于存储方块的动态状态，如方向、含水等）
        uint8_t GetStateData() const { return m_stateData; }
        void    SetStateData(uint8_t data) { m_stateData = data; }

        // 便捷方法
        bool        IsAir() const { return m_definition == nullptr; }
        std::string GetName() const { return m_definition ? m_definition->GetName() : "air"; }

        // 比较操作符
        bool operator==(const Block& other) const { return m_definition == other.m_definition && m_stateData == other.m_stateData; }
        bool operator!=(const Block& other) const { return !(*this == other); }

        // 静态便捷方法
        static Block Air() { return Block(); }

    private:
        const BlockDefinition* m_definition; // 指向共享的BlockDefinition (Flyweight)
        uint8_t                m_stateData; // 方块状态数据（8位足够存储大多数状态）

        // 如果需要更复杂的状态，可以改为：
        // std::shared_ptr<BlockStateData> m_stateData;
    };

    // 类型别名
    using BlockPtr = std::shared_ptr<Block>;
}
