#pragma once
#include "Engine/Resource/ResourceCommon.hpp"
#include "Engine/Core/Json.hpp"
#include <string>
#include <unordered_map>

namespace simpleminer::framework::block
{
    using namespace enigma::resource;
    using namespace enigma::core;

    struct BlockStateVariant
    {
        std::string model;   // Model resource path like "simpleminer:block/grass"
        int x = 0;          // X rotation (optional)
        int y = 0;          // Y rotation (optional)
        bool uvlock = false; // UV lock (optional)
        
        BlockStateVariant() = default;
        BlockStateVariant(const std::string& modelPath) : model(modelPath) {}
    };

    class BlockState
    {
    public:
        BlockState() = default;
        BlockState(const ResourceLocation& location) : m_location(location) {}

        // Load from JSON
        bool LoadFromJson(const JsonObject& json);
        bool LoadFromFile(const std::filesystem::path& filePath);

        // Access variants
        const std::unordered_map<std::string, BlockStateVariant>& GetVariants() const { return m_variants; }
        const BlockStateVariant* GetVariant(const std::string& variantKey = "") const;
        
        // Default variant access (empty string key)
        const BlockStateVariant* GetDefaultVariant() const { return GetVariant(""); }

        // Resource location
        const ResourceLocation& GetLocation() const { return m_location; }
        void SetLocation(const ResourceLocation& location) { m_location = location; }

        // Check if loaded
        bool IsLoaded() const { return m_loaded; }

        // Factory methods
        static std::shared_ptr<BlockState> Create(const ResourceLocation& location);
        static std::shared_ptr<BlockState> LoadFromFile(const ResourceLocation& location, const std::filesystem::path& filePath);

    private:
        ResourceLocation m_location;
        std::unordered_map<std::string, BlockStateVariant> m_variants;
        bool m_loaded = false;
    };

    using BlockStatePtr = std::shared_ptr<BlockState>;
}
