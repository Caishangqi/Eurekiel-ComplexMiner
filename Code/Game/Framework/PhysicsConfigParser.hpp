#pragma once
#include <string>

//-----------------------------------------------------------------------------------------------
// PhysicsConfigParser.hpp
// Loads and validates physics configuration from YAML files.
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// PhysicsConfig - Physics tuning parameters for Entity physics simulation
// All parameters have sensible defaults, loaded from settings.yml if available
//-----------------------------------------------------------------------------------------------
struct PhysicsConfig
{
    float m_gravityConstant         = 9.8f; // Gravity constant (m/s^2)
    float m_groundedDragCoefficient = 8.0f; // Ground drag coefficient (friction when touching ground)
    float m_airborneDragCoefficient = 0.5f; // Airborne drag coefficient (air resistance)
    float m_groundedAcceleration    = 10.0f; // Ground acceleration (m/s^2)
    float m_airborneAcceleration    = 2.0f; // Airborne acceleration (m/s^2)
    float m_speedLimit              = 10.0f; // Maximum horizontal speed (m/s)
    float m_jumpImpulse             = 5.0f; // Jump instant velocity impulse (m/s)
};

//-----------------------------------------------------------------------------------------------
// PhysicsConfigParser - Utility class for loading physics configuration from YAML
// Provides validation to ensure all parameters are within reasonable ranges
//-----------------------------------------------------------------------------------------------
class PhysicsConfigParser
{
public:
    /// Loads physics configuration from YAML file
    /// @param yamlPath Path to YAML configuration file (e.g., "Run/.enigma/settings.yml")
    /// @return PhysicsConfig with loaded values, or defaults if loading fails
    static PhysicsConfig LoadFromYaml(const std::string& yamlPath);

    /// Validates physics configuration parameters
    /// @param config Configuration to validate
    /// @return true if all parameters are within valid ranges, false otherwise
    static bool ValidateConfig(const PhysicsConfig& config);
};
