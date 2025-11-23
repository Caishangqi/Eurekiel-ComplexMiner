#include "PhysicsConfigParser.hpp"
#include "Engine/Core/Yaml.hpp"
#include "Engine/Core/EngineCommon.hpp"

using namespace enigma::core;

PhysicsConfig PhysicsConfigParser::LoadFromYaml(const std::string& yamlPath)
{
    PhysicsConfig config;

    try
    {
        auto yamlConfig = YamlConfiguration::LoadFromFile(yamlPath);

        DebuggerPrintf("Loading physics config from: %s\n", yamlPath.c_str());

        // Parse physics parameters
        config.m_gravityConstant         = yamlConfig.GetFloat("physics.gravityConstant", 9.8f);
        config.m_groundedDragCoefficient = yamlConfig.GetFloat("physics.groundedDragCoefficient", 8.0f);
        config.m_airborneDragCoefficient = yamlConfig.GetFloat("physics.airborneDragCoefficient", 0.5f);
        config.m_groundedAcceleration    = yamlConfig.GetFloat("physics.groundedAcceleration", 10.0f);
        config.m_airborneAcceleration    = yamlConfig.GetFloat("physics.airborneAcceleration", 2.0f);
        config.m_speedLimit              = yamlConfig.GetFloat("physics.speedLimit", 10.0f);
        config.m_jumpImpulse             = yamlConfig.GetFloat("physics.jumpImpulse", 5.0f);

        DebuggerPrintf("Parsed physics config:\n");
        DebuggerPrintf("  Gravity: %f\n", config.m_gravityConstant);
        DebuggerPrintf("  Grounded Drag: %f\n", config.m_groundedDragCoefficient);
        DebuggerPrintf("  Airborne Drag: %f\n", config.m_airborneDragCoefficient);
        DebuggerPrintf("  Grounded Accel: %f\n", config.m_groundedAcceleration);
        DebuggerPrintf("  Airborne Accel: %f\n", config.m_airborneAcceleration);
        DebuggerPrintf("  Speed Limit: %f\n", config.m_speedLimit);
        DebuggerPrintf("  Jump Impulse: %f\n", config.m_jumpImpulse);

        // Validate configuration
        if (!ValidateConfig(config))
        {
            DebuggerPrintf("Warning: Invalid physics configuration detected, using defaults\n");
        }
    }
    catch (const std::exception& e)
    {
        DebuggerPrintf("Error loading physics config from %s: %s\n", yamlPath.c_str(), e.what());
        DebuggerPrintf("Using default physics configuration\n");
        // Use default values (already set in struct)
    }

    return config;
}

bool PhysicsConfigParser::ValidateConfig(const PhysicsConfig& config)
{
    // Validate gravity constant (must be positive)
    if (config.m_gravityConstant <= 0.0f || config.m_gravityConstant > 50.0f)
    {
        DebuggerPrintf("Invalid gravity constant: %f (expected 0.1-50.0)\n", config.m_gravityConstant);
        return false;
    }

    // Validate drag coefficients (must be non-negative)
    if (config.m_groundedDragCoefficient < 0.0f || config.m_groundedDragCoefficient > 20.0f)
    {
        DebuggerPrintf("Invalid grounded drag coefficient: %f (expected 0.0-20.0)\n", config.m_groundedDragCoefficient);
        return false;
    }

    if (config.m_airborneDragCoefficient < 0.0f || config.m_airborneDragCoefficient > 10.0f)
    {
        DebuggerPrintf("Invalid airborne drag coefficient: %f (expected 0.0-10.0)\n", config.m_airborneDragCoefficient);
        return false;
    }

    // Validate acceleration values (must be positive)
    if (config.m_groundedAcceleration <= 0.0f || config.m_groundedAcceleration > 50.0f)
    {
        DebuggerPrintf("Invalid grounded acceleration: %f (expected 0.1-50.0)\n", config.m_groundedAcceleration);
        return false;
    }

    if (config.m_airborneAcceleration <= 0.0f || config.m_airborneAcceleration > 20.0f)
    {
        DebuggerPrintf("Invalid airborne acceleration: %f (expected 0.1-20.0)\n", config.m_airborneAcceleration);
        return false;
    }

    // Validate speed limit (must be positive)
    if (config.m_speedLimit <= 0.0f || config.m_speedLimit > 100.0f)
    {
        DebuggerPrintf("Invalid speed limit: %f (expected 0.1-100.0)\n", config.m_speedLimit);
        return false;
    }

    // Validate jump impulse (must be positive)
    if (config.m_jumpImpulse <= 0.0f || config.m_jumpImpulse > 20.0f)
    {
        DebuggerPrintf("Invalid jump impulse: %f (expected 0.1-20.0)\n", config.m_jumpImpulse);
        return false;
    }

    return true;
}
