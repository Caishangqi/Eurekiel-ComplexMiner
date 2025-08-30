#pragma once
#include <string>
#include <memory>
#include "Engine/Resource/ResourceCommon.hpp"
#include "Engine/Core/Yaml.hpp"

namespace simpleminer::framework::block
{
    using namespace enigma::resource;
    using namespace enigma::core;

    /**
     * BlockDefinition - Flyweight模式，存储方块的共享属性和行为
     * 类似于Minecraft的Block类，是注册到Registry中的对象
     */
    class BlockDefinition
    {
    public:
        BlockDefinition() = default;
        BlockDefinition(const ResourceLocation& location);
        BlockDefinition(const ResourceLocation& location, const YamlConfiguration& config);
        virtual ~BlockDefinition() = default;

        // Basic properties from Assignment01
        bool IsVisible() const { return m_isVisible; }
        bool IsSolid() const { return m_isSolid; }
        bool IsOpaque() const { return m_isOpaque; }
        int GetIndoorLighting() const { return m_indoorLighting; }


        // Resource location and name
        const ResourceLocation& GetLocation() const { return m_location; }
        const std::string& GetName() const { return m_name; }

        // BlockState resource path (for new system)
        std::string GetBlockStatePath() const;

        // Setters for builder pattern
        BlockDefinition& SetVisible(bool visible) { m_isVisible = visible; return *this; }
        BlockDefinition& SetSolid(bool solid) { m_isSolid = solid; return *this; }
        BlockDefinition& SetOpaque(bool opaque) { m_isOpaque = opaque; return *this; }
        BlockDefinition& SetIndoorLighting(int lighting) { m_indoorLighting = lighting; return *this; }
        BlockDefinition& SetName(const std::string& name) { m_name = name; return *this; }
        
        // Minecraft-style methods
        virtual ResourceLocation GetBlockStateLocation() const;
        virtual bool CanConnectTo(const BlockDefinition* other) const { return false; }
        virtual int GetLightEmission() const { return m_indoorLighting; }
        virtual float GetHardness() const { return 1.0f; }
        virtual float getExplosionResistance() const { return 1.0f; }

        // Factory method for registration
        static std::shared_ptr<BlockDefinition> Create(const ResourceLocation& location);

        // YAML factory (for registry system)
        static std::shared_ptr<BlockDefinition> CreateFromYaml(const ResourceLocation& location, const YamlConfiguration& config);

    protected:
        // Load from YAML configuration
        virtual void LoadFromYaml(const YamlConfiguration& config);
        ResourceLocation m_location;
        std::string m_name;
        
        // Properties from Assignment01 and Demo
        bool m_isVisible = true;
        bool m_isSolid = true;
        bool m_isOpaque = true;
        int m_indoorLighting = 0; // 0-15, for blocks like glowstone
    };

    using BlockDefinitionPtr = std::shared_ptr<BlockDefinition>;
}
