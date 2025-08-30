#include "ModelResource.hpp"
#include <fstream>

namespace simpleminer::framework::graphic
{
    ModelResource::ModelResource(const ResourceMetadata& metadata)
        : m_metadata(metadata)
    {
    }

    bool ModelResource::LoadFromJson(const JsonObject& json)
    {
        try
        {
            if (json.ContainsKey("parent"))
            {
                m_parent = json.GetString("parent");
            }

            if (json.ContainsKey("textures"))
            {
                auto textures = json.GetJsonObject("textures");
                
                for (const auto& [key, value] : textures.GetJson().items())
                {
                    if (value.is_string())
                    {
                        m_textures[key] = value.get<std::string>();
                    }
                }
            }

            ResolveTextureReferences();
            m_loaded = true;
            return true;
        }
        catch (const std::exception&)
        {
            m_loaded = false;
            return false;
        }
    }

    bool ModelResource::LoadFromFile(const std::filesystem::path& filePath)
    {
        try
        {
            std::ifstream file(filePath);
            if (!file.is_open())
            {
                return false;
            }

            std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            JsonObject json = JsonObject::Parse(jsonContent);
            return LoadFromJson(json);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string ModelResource::GetTexture(const std::string& key) const
    {
        auto it = m_textures.find(key);
        if (it != m_textures.end())
        {
            return it->second;
        }
        return "";
    }

    bool ModelResource::HasTexture(const std::string& key) const
    {
        return m_textures.find(key) != m_textures.end();
    }

    std::shared_ptr<ModelResource> ModelResource::Create(const ResourceLocation& location)
    {
        ResourceMetadata metadata;
        metadata.location = location;
        metadata.type = ResourceType::MODEL;
        return std::make_shared<ModelResource>(metadata);
    }

    std::shared_ptr<ModelResource> ModelResource::LoadFromFile(const ResourceLocation& location, const std::filesystem::path& filePath)
    {
        auto modelResource = Create(location);
        modelResource->LoadFromFile(filePath);
        return modelResource;
    }

    void ModelResource::ResolveTextureReferences()
    {
        std::unordered_map<std::string, std::string> resolved;
        
        for (auto& [key, value] : m_textures)
        {
            if (!value.empty() && value.front() == '#')
            {
                std::string refKey = value.substr(1);
                auto refIt = m_textures.find(refKey);
                if (refIt != m_textures.end() && !refIt->second.empty() && refIt->second.front() != '#')
                {
                    resolved[key] = refIt->second;
                }
            }
            else
            {
                resolved[key] = value;
            }
        }
        
        for (const auto& [key, value] : resolved)
        {
            m_textures[key] = value;
        }
    }
}