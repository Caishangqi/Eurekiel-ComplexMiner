#include "ControlConfigParser.hpp"
#include "Engine/Core/Yaml.hpp"
#include "Engine/Core/EngineCommon.hpp"

using namespace enigma::core;

ControlConfig ControlConfigParser::LoadFromYaml(const std::string& yamlPath)
{
    ControlConfig config;

    try
    {
        auto yamlConfig = YamlConfiguration::LoadFromFile(yamlPath);

        DebuggerPrintf("Loading control config from: %s\n", yamlPath.c_str());

        // Parse control parameters
        config.m_mouseSensitivity = yamlConfig.GetFloat("control.mouseSensitivity", 0.075f);

        DebuggerPrintf("Parsed control config:\n");
        DebuggerPrintf("  Mouse Sensitivity: %f\n", config.m_mouseSensitivity);

        // Validate configuration
        if (!ValidateConfig(config))
        {
            DebuggerPrintf("Warning: Invalid control configuration detected, using defaults\n");
        }
    }
    catch (const std::exception& e)
    {
        DebuggerPrintf("Error loading control config from %s: %s\n", yamlPath.c_str(), e.what());
        DebuggerPrintf("Using default control configuration\n");
        // Use default values (already set in struct)
    }

    return config;
}

bool ControlConfigParser::ValidateConfig(const ControlConfig& config)
{
    // Validate mouse sensitivity (must be positive and reasonable)
    if (config.m_mouseSensitivity <= 0.0f || config.m_mouseSensitivity > 1.0f)
    {
        DebuggerPrintf("Invalid mouse sensitivity: %f (expected 0.001-1.0)\n", config.m_mouseSensitivity);
        return false;
    }

    return true;
}
