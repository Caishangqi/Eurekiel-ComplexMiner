#pragma once
#include "Engine/Resource/ResourceMetadata.hpp"
#include "Engine/Core/Json.hpp"
#include <unordered_map>

namespace simpleminer::framework::graphic
{
    using namespace enigma::resource;
    using namespace enigma::core;

    class ModelResource : public IResource
    {
    public:
        ModelResource(const ResourceMetadata& metadata);
        virtual ~ModelResource() = default;

        // IResource interface
        const ResourceMetadata& GetMetadata() const override { return m_metadata; }
        ResourceType GetType() const override { return ResourceType::MODEL; }
        bool IsLoaded() const override { return m_loaded; }

        // Load from JSON
        bool LoadFromJson(const JsonObject& json);
        bool LoadFromFile(const std::filesystem::path& filePath);

        // Model properties
        const std::string& GetParent() const { return m_parent; }
        const std::unordered_map<std::string, std::string>& GetTextures() const { return m_textures; }
        
        // Texture lookup
        std::string GetTexture(const std::string& key) const;
        bool HasTexture(const std::string& key) const;

        // Factory methods
        static std::shared_ptr<ModelResource> Create(const ResourceLocation& location);
        static std::shared_ptr<ModelResource> LoadFromFile(const ResourceLocation& location, const std::filesystem::path& filePath);

    private:
        ResourceMetadata m_metadata;
        std::string m_parent = "block/cube";  // Default parent model
        std::unordered_map<std::string, std::string> m_textures; // Key -> texture path mapping
        bool m_loaded = false;

        // Resolve texture references (handles parent inheritance)
        void ResolveTextureReferences();
    };

    using ModelResourcePtr = std::shared_ptr<ModelResource>;
}
