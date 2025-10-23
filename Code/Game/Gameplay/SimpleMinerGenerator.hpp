#pragma once
#include "Engine/Voxel/Generation/Generator.hpp"
#include "Engine/Math/SmoothNoise.hpp"
#include "Engine/Math/RawNoise.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Engine.hpp"
#include <unordered_map>
#include <memory>

#include "Engine/Core/LogCategory/LogCategory.hpp"

DECLARE_LOG_CATEGORY_EXTERN(LogWorldGenerator)
using namespace enigma::voxel;
/**
 * @brief SimpleMiner world generator implementation
 *
 * Implements the Assignment 02 chunk generation algorithm using Perlin noise
 * for terrain, humidity, temperature, and other biome characteristics.
 * Based on the provided pseudocode from Assignment 02 specification.
 */
class SimpleMinerGenerator : public Generator
{
private:
    // Static constants from Assignment 02 specification
    static constexpr unsigned int GAME_SEED = 0u;

    static constexpr float DEFAULT_OCTAVE_PERSISTANCE = 0.5f;
    static constexpr float DEFAULT_NOISE_OCTAVE_SCALE = 2.0f;

    static constexpr float        DEFAULT_TERRAIN_HEIGHT = 64.0f;
    static constexpr float        RIVER_DEPTH            = 8.0f;
    static constexpr float        TERRAIN_NOISE_SCALE    = 200.0f;
    static constexpr unsigned int TERRAIN_NOISE_OCTAVES  = 5u;

    static constexpr float        HUMIDITY_NOISE_SCALE   = 800.0f;
    static constexpr unsigned int HUMIDITY_NOISE_OCTAVES = 4u;

    static constexpr float        TEMPERATURE_RAW_NOISE_SCALE = 0.0075f;
    static constexpr float        TEMPERATURE_NOISE_SCALE     = 400.0f;
    static constexpr unsigned int TEMPERATURE_NOISE_OCTAVES   = 4u;

    static constexpr float        HILLINESS_NOISE_SCALE   = 250.0f;
    static constexpr unsigned int HILLINESS_NOISE_OCTAVES = 4u;

    static constexpr float OCEAN_START_THRESHOLD = 0.0f;
    static constexpr float OCEAN_END_THRESHOLD   = 0.5f;
    static constexpr float OCEAN_DEPTH           = 30.0f;

    static constexpr float        OCEANESS_NOISE_SCALE   = 600.0f;
    static constexpr unsigned int OCEANESS_NOISE_OCTAVES = 3u;

    static constexpr int   MIN_DIRT_OFFSET_Z = 3;
    static constexpr int   MAX_DIRT_OFFSET_Z = 4;
    static constexpr float MIN_SAND_HUMIDITY = 0.4f;
    static constexpr float MAX_SAND_HUMIDITY = 0.7f;
    static constexpr int   SEA_LEVEL_Z       = Chunk::CHUNK_SIZE_Z / 2; // 64

    static constexpr float ICE_TEMPERATURE_MAX = 0.37f;
    static constexpr float ICE_TEMPERATURE_MIN = 0.0f;
    static constexpr float ICE_DEPTH_MIN       = 0.0f;
    static constexpr float ICE_DEPTH_MAX       = 8.0f;

    static constexpr float MIN_SAND_DEPTH_HUMIDITY = 0.4f;
    static constexpr float MAX_SAND_DEPTH_HUMIDITY = 0.0f;
    static constexpr float SAND_DEPTH_MIN          = 0.0f;
    static constexpr float SAND_DEPTH_MAX          = 6.0f;

    static constexpr float COAL_CHANCE    = 0.05f;
    static constexpr float IRON_CHANCE    = 0.02f;
    static constexpr float GOLD_CHANCE    = 0.005f;
    static constexpr float DIAMOND_CHANCE = 0.0001f;
    static constexpr int   OBSIDIAN_Z     = 1;
    static constexpr int   LAVA_Z         = 0;

    // Current world seed
    uint32_t m_worldSeed = GAME_SEED;

    // Performance optimization: cache block IDs to avoid repeated registry lookups
    mutable std::unordered_map<std::string, int>                                     m_blockIdCache;
    mutable std::unordered_map<int, std::shared_ptr<enigma::registry::block::Block>> m_blockByIdCache;

    // Pre-cached Block IDs for fast generation (populated during initialization)
    mutable int m_airId        = -1;
    mutable int m_grassId      = -1;
    mutable int m_dirtId       = -1;
    mutable int m_stoneId      = -1;
    mutable int m_sandId       = -1;
    mutable int m_waterId      = -1;
    mutable int m_iceId        = -1;
    mutable int m_lavaId       = -1;
    mutable int m_obsidianId   = -1;
    mutable int m_coalOreId    = -1;
    mutable int m_ironOreId    = -1;
    mutable int m_goldOreId    = -1;
    mutable int m_diamondOreId = -1;

public:
    SimpleMinerGenerator()
        : Generator("simpleminer_generator", "simpleminer")
    {
    }

    virtual ~SimpleMinerGenerator() = default;

    // Generator interface implementation
    void GenerateChunk(Chunk* chunk, int32_t chunkX, int32_t chunkZ, uint32_t worldSeed) override;

    int32_t GetSeaLevel() const override { return SEA_LEVEL_Z; }
    int32_t GetBaseHeight() const override { return static_cast<int32_t>(DEFAULT_TERRAIN_HEIGHT); }

    std::string GetConfigDescription() const override
    {
        return "SimpleMiner perlin noise terrain generator with biomes, rivers, and ores";
    }

    bool Initialize(uint32_t seed) override
    {
        m_worldSeed = seed;
        InitializeBlockCache();
        return true;
    }

    std::string GetDisplayName() const override { return "SimpleMiner Generator"; }

    std::string GetDescription() const override
    {
        return "Generates varied terrain with humidity/temperature-based biomes, rivers, oceans, and underground ores";
    }

    bool SupportsFeature(const std::string& featureName) const override
    {
        return featureName == "biomes" ||
            featureName == "rivers" ||
            featureName == "oceans" ||
            featureName == "ores" ||
            featureName == "caves";
    }

private:
    // Utility functions
    float RangeMap(float value, float inMin, float inMax, float outMin, float outMax) const;
    float RangeMapClamped(float value, float inMin, float inMax, float outMin, float outMax) const;
    float Lerp(float a, float b, float t) const;
    float SmoothStep3(float t) const;

    // Noise wrapper functions
    float ComputePerlin2D(float x, float           y, float          scale, unsigned int octaves,
                          float persistence, float octaveScale, bool wrap, unsigned int  seed) const;

    // Block type determination - optimized to return block ID directly
    int DetermineBlockTypeId(const IntVec3& globalPos, int  terrainHeight, int dirtDepth,
                             float          humidity, float temperature, float iceDepth) const;

    int DetermineOreTypeId(const IntVec3& globalPos) const;

    // Legacy string-based methods (kept for compatibility)
    std::string DetermineBlockType(const IntVec3& globalPos, int  terrainHeight, int dirtDepth,
                                   float          humidity, float temperature, float iceDepth) const;

    std::string DetermineOreType(const IntVec3& globalPos) const;

    // Performance optimization: cached block lookups using numeric IDs
    void                                            InitializeBlockCache() const;
    std::shared_ptr<enigma::registry::block::Block> GetCachedBlock(const std::string& blockName) const;
    std::shared_ptr<enigma::registry::block::Block> GetCachedBlockById(int blockId) const;
};
