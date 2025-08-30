#include "BlockDefinition.hpp"

namespace simpleminer::framework::block
{
    BlockDefinition::BlockDefinition(const ResourceLocation& location)
        : m_location(location)
          , m_name(location.GetPath())
    {
    }

    BlockDefinition::BlockDefinition(const ResourceLocation& location, const YamlConfiguration& config)
        : m_location(location)
          , m_name(location.GetPath())
    {
        BlockDefinition::LoadFromYaml(config);
    }

    ResourceLocation BlockDefinition::GetBlockStateLocation() const
    {
        // 返回 BlockState 资源的位置
        return ResourceLocation(m_location.GetNamespace(), "blockstates/" + m_location.GetPath());
    }

    std::string BlockDefinition::GetBlockStatePath() const
    {
        // 返回文件系统路径
        return "assets/" + m_location.GetNamespace() + "/blockstates/" + m_location.GetPath() + ".json";
    }

    std::shared_ptr<BlockDefinition> BlockDefinition::Create(const ResourceLocation& location)
    {
        return std::make_shared<BlockDefinition>(location);
    }

    std::shared_ptr<BlockDefinition> BlockDefinition::CreateFromYaml(const ResourceLocation& location, const YamlConfiguration& config)
    {
        return std::make_shared<BlockDefinition>(location, config);
    }

    void BlockDefinition::LoadFromYaml(const YamlConfiguration& config)
    {
        // 解析 YAML 配置，按照你的 grass.yml 格式
        if (config.Contains("block.properties.isVisible"))
            m_isVisible = config.GetBoolean("block.properties.isVisible");
        if (config.Contains("block.properties.isSolid"))
            m_isSolid = config.GetBoolean("block.properties.isSolid");
        if (config.Contains("block.properties.isOpaque"))
            m_isOpaque = config.GetBoolean("block.properties.isOpaque");
        if (config.Contains("block.properties.indoorLighting"))
            m_indoorLighting = config.GetInt("block.properties.indoorLighting");

        // 设置名称
        if (config.Contains("block.name"))
            m_name = config.GetString("block.name", m_location.GetPath());
        else
            m_name = m_location.GetPath();

        // BlockState 信息已经由 GetBlockStateLocation() 处理
    }
}
