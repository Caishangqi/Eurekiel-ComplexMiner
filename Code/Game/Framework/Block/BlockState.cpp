#include "BlockState.hpp"
#include <fstream>

namespace simpleminer::framework::block
{
    bool BlockState::LoadFromJson(const JsonObject& json)
    {
        try
        {
            if (!json.ContainsKey("variants"))
            {
                return false;
            }

            auto variants = json.GetJsonObject("variants");
            
            // Parse all variants
            for (const auto& [key, value] : variants.GetJson().items())
            {
                BlockStateVariant variant;
                
                if (value.contains("model") && value["model"].is_string())
                {
                    variant.model = value["model"].get<std::string>();
                }
                
                if (value.contains("x") && value["x"].is_number())
                {
                    variant.x = value["x"].get<int>();
                }
                
                if (value.contains("y") && value["y"].is_number())
                {
                    variant.y = value["y"].get<int>();
                }
                
                if (value.contains("uvlock") && value["uvlock"].is_boolean())
                {
                    variant.uvlock = value["uvlock"].get<bool>();
                }
                
                m_variants[key] = variant;
            }

            m_loaded = true;
            return true;
        }
        catch (const std::exception&)
        {
            m_loaded = false;
            return false;
        }
    }

    bool BlockState::LoadFromFile(const std::filesystem::path& filePath)
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

    const BlockStateVariant* BlockState::GetVariant(const std::string& variantKey) const
    {
        auto it = m_variants.find(variantKey);
        if (it != m_variants.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    std::shared_ptr<BlockState> BlockState::Create(const ResourceLocation& location)
    {
        return std::make_shared<BlockState>(location);
    }

    std::shared_ptr<BlockState> BlockState::LoadFromFile(const ResourceLocation& location, const std::filesystem::path& filePath)
    {
        auto blockState = std::make_shared<BlockState>(location);
        blockState->LoadFromFile(filePath);
        return blockState;
    }
}
