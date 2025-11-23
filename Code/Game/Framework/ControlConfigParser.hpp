#pragma once
#include <string>

//-----------------------------------------------------------------------------------------------
// ControlConfigParser.hpp
// Loads and validates control configuration from YAML files.
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// ControlConfig - Input control parameters for player and camera control
// All parameters have sensible defaults, loaded from settings.yml if available
//-----------------------------------------------------------------------------------------------
struct ControlConfig
{
    float m_mouseSensitivity = 0.075f; // Mouse sensitivity for camera control (degrees/pixel)
};

//-----------------------------------------------------------------------------------------------
// ControlConfigParser - Utility class for loading control configuration from YAML
// Provides validation to ensure all parameters are within reasonable ranges
//-----------------------------------------------------------------------------------------------
class ControlConfigParser
{
public:
    /// Loads control configuration from YAML file
    /// @param yamlPath Path to YAML configuration file (e.g., "Run/.enigma/settings.yml")
    /// @return ControlConfig with loaded values, or defaults if loading fails
    static ControlConfig LoadFromYaml(const std::string& yamlPath);

    /// Validates control configuration parameters
    /// @param config Configuration to validate
    /// @return true if all parameters are within valid ranges, false otherwise
    static bool ValidateConfig(const ControlConfig& config);
};
