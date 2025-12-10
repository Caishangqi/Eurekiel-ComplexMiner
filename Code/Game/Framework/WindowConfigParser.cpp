#include "WindowConfigParser.hpp"
#include "Engine/Core/Yaml.hpp"
#include "Engine/Core/EngineCommon.hpp"

using namespace enigma::core;

const std::unordered_map<std::string, WindowMode> WindowConfigParser::s_windowModeMap = {
    {"windowed", WindowMode::Windowed},
    {"fullscreen", WindowMode::Fullscreen},
    {"borderlessFullscreen", WindowMode::BorderlessFullscreen}
};

WindowConfig WindowConfigParser::LoadFromYaml(const std::string& yamlPath)
{
    WindowConfig config;

    try
    {
        auto yamlConfig = YamlConfiguration::LoadFromFile(yamlPath);

        // Debug output: Confirm configuration file loading
        DebuggerPrintf("Loading window config from: %s\n", yamlPath.c_str());

        // Parse window mode
        std::string windowModeStr = yamlConfig.GetString("video.windowMode", "windowed");
        config.m_windowMode       = ParseWindowMode(windowModeStr);
        DebuggerPrintf("Parsed window mode: %s -> %d\n", windowModeStr.c_str(), (int)config.m_windowMode);

        // parse resolution
        config.m_resolution = ParseResolution(yamlPath);
        DebuggerPrintf("Parsed resolution: %dx%d\n", config.m_resolution.x, config.m_resolution.y);

        // Parse aspect ratio
        config.m_aspectRatio = ParseAspectRatio(yamlPath);
        DebuggerPrintf("Parsed aspect ratio: %f\n", config.m_aspectRatio);

        // Parse window title
        std::string appName  = yamlConfig.GetString("general.appName", "SimpleMiner");
        config.m_windowTitle = appName;
        DebuggerPrintf("Parsed window title: %s\n", config.m_windowTitle.c_str());

        // Parse Always On Top settings
        bool alwaysOnTop     = yamlConfig.GetBoolean("video.window.alwaysOnTop", false);
        config.m_alwaysOnTop = alwaysOnTop;
        DebuggerPrintf("Parsed always on top: %s\n", alwaysOnTop ? "true" : "false");

        //Verify configuration
        if (!ValidateConfig(config))
        {
            DebuggerPrintf("Warning: Invalid window configuration detected, using defaults\n");
        }
    }
    catch (const std::exception& e)
    {
        DebuggerPrintf("Error loading window config from %s: %s\n", yamlPath.c_str(), e.what());
        DebuggerPrintf("Using default window configuration\n");
        // Make sure to use sensible defaults
        config.m_windowTitle = "SimpleMiner";
        config.m_windowMode  = WindowMode::Windowed;
        config.m_resolution  = IntVec2(1600, 900);
        config.m_aspectRatio = 16.0f / 9.0f;
        config.m_alwaysOnTop = false;
    }

    return config;
}

WindowMode WindowConfigParser::ParseWindowMode(const std::string& modeString)
{
    auto it = s_windowModeMap.find(modeString);
    if (it != s_windowModeMap.end())
    {
        return it->second;
    }

    DebuggerPrintf("Warning: Unknown window mode '%s', defaulting to windowed\n", modeString.c_str());
    return WindowMode::Windowed;
}

IntVec2 WindowConfigParser::ParseResolution(const std::string& configPath)
{
    IntVec2 resolution(1600, 900); // 默认分辨率

    try
    {
        auto yamlConfig = YamlConfiguration::LoadFromFile(configPath);

        // Use the repaired dot syntax to directly obtain the resolution
        if (yamlConfig.Contains("video.resolution.width") && yamlConfig.Contains("video.resolution.height"))
        {
            int width  = yamlConfig.GetInt("video.resolution.width", 1600);
            int height = yamlConfig.GetInt("video.resolution.height", 900);

            if (width >= 640 && width <= 7680 && height >= 480 && height <= 4320)
            {
                resolution = IntVec2(width, height);
                DebuggerPrintf("Loaded resolution from YAML config: %dx%d\n", width, height);
                return resolution;
            }
        }

        // Fallback to direct yaml-cpp (alternative solution)
        YAML::Node yamlNode = YAML::LoadFile(configPath);

        if (yamlNode["video"] && yamlNode["video"]["resolution"])
        {
            const YAML::Node& resolutionNode = yamlNode["video"]["resolution"];

            if (resolutionNode["width"] && resolutionNode["height"])
            {
                int width  = resolutionNode["width"].as<int>(1600);
                int height = resolutionNode["height"].as<int>(900);

                if (width >= 640 && width <= 7680 && height >= 480 && height <= 4320)
                {
                    resolution = IntVec2(width, height);
                    DebuggerPrintf("Loaded resolution using fallback yaml-cpp: %dx%d\n", width, height);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        DebuggerPrintf("Error parsing resolution: %s\n", e.what());
    }

    return resolution;
}

float WindowConfigParser::ParseAspectRatio(const std::string& configPath)
{
    float aspectRatio = 16.0f / 9.0f; //Default aspect ratio

    try
    {
        // Use yaml-cpp directly to avoid problems with YamlConfiguration
        YAML::Node yamlNode = YAML::LoadFile(configPath);

        if (yamlNode["video"] && yamlNode["video"]["aspectRatio"])
        {
            aspectRatio = yamlNode["video"]["aspectRatio"].as<float>(16.0f / 9.0f);
            DebuggerPrintf("Direct yaml-cpp loading aspect ratio: %f\n", aspectRatio);
        }
        else
        {
            // If there is no explicit aspect ratio, calculate it from the resolution
            IntVec2 resolution = ParseResolution(configPath);
            if (resolution.y > 0)
            {
                aspectRatio = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
                DebuggerPrintf("Calculate aspect ratio from resolution: %f\n", aspectRatio);
            }
        }
    }
    catch (const YAML::Exception& e)
    {
        DebuggerPrintf("Error parsing aspect ratio from %s: %s\n", configPath.c_str(), e.what());

        // If parsing fails, try calculating from resolution
        IntVec2 resolution = ParseResolution(configPath);
        if (resolution.y > 0)
        {
            aspectRatio = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
            DebuggerPrintf("Fallback: Calculate aspect ratio from resolution: %f\n", aspectRatio);
        }
    }

    return aspectRatio;
}

bool WindowConfigParser::ValidateConfig(const WindowConfig& config)
{
    // Verify resolution
    if (config.m_resolution.x < 640 || config.m_resolution.x > 7680 ||
        config.m_resolution.y < 480 || config.m_resolution.y > 4320)
    {
        return false;
    }

    // Verify aspect ratio
    if (config.m_aspectRatio <= 0.0f || config.m_aspectRatio > 10.0f)
    {
        return false;
    }

    // Verify window title
    if (config.m_windowTitle.empty())
    {
        return false;
    }

    return true;
}
