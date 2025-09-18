#include "SimpleMinerGenerator.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
#include "Engine/Core/Logger/LoggerAPI.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Voxel/Block/BlockPos.hpp"
#include <cmath>
#include <algorithm>

#include "Engine/Math/IntVec3.hpp"

using namespace enigma::registry::block;
using namespace enigma::voxel::block;

void SimpleMinerGenerator::GenerateChunk(Chunk* chunk, int32_t chunkX, int32_t chunkZ, uint32_t worldSeed)
{
    if (!chunk)
    {
        LogError("SimpleMinerGenerator", "GenerateChunk - null chunk provided");
        return;
    }

    // Use provided world seed or fallback to member seed
    uint32_t effectiveSeed = (worldSeed != 0) ? worldSeed : m_worldSeed;

    // Establish world-space position and bounds of this chunk
    IntVec3 chunkPosition(chunkX * Chunk::CHUNK_SIZE_X, chunkZ * Chunk::CHUNK_SIZE_Y, 0);

    // Derive deterministic seeds for each noise channel
    unsigned int terrainSeed     = effectiveSeed;
    unsigned int humiditySeed    = effectiveSeed + 1;
    unsigned int temperatureSeed = humiditySeed + 1;
    unsigned int hillSeed        = temperatureSeed + 1;
    unsigned int oceanSeed       = hillSeed + 1;
    unsigned int dirtSeed        = oceanSeed + 1;

    // Allocate per-(x,y) maps
    const int          mapSize = Chunk::CHUNK_SIZE_X * Chunk::CHUNK_SIZE_Y;
    std::vector<int>   heightMapXY(mapSize);
    std::vector<int>   dirtDepthXY(mapSize);
    std::vector<float> humidityMapXY(mapSize);
    std::vector<float> temperatureMapXY(mapSize);

    // --- Pass 1: compute surface & biome fields per (x,y) pillar ---
    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; ++y)
    {
        for (int x = 0; x < Chunk::CHUNK_SIZE_X; ++x)
        {
            int globalX = chunkX * Chunk::CHUNK_SIZE_X + x;
            int globalY = chunkZ * Chunk::CHUNK_SIZE_Y + y;

            // Humidity computation
            float humidity = 0.5f + 0.5f * ComputePerlin2D(
                static_cast<float>(globalX), static_cast<float>(globalY),
                HUMIDITY_NOISE_SCALE,
                HUMIDITY_NOISE_OCTAVES,
                DEFAULT_OCTAVE_PERSISTANCE,
                DEFAULT_NOISE_OCTAVE_SCALE,
                true,
                humiditySeed
            );

            // Temperature computation
            float temperature = Get2dNoiseNegOneToOne(globalX, globalY, temperatureSeed) * TEMPERATURE_RAW_NOISE_SCALE;
            temperature       = temperature + 0.5f + 0.5f * ComputePerlin2D(
                static_cast<float>(globalX), static_cast<float>(globalY),
                TEMPERATURE_NOISE_SCALE,
                TEMPERATURE_NOISE_OCTAVES,
                DEFAULT_OCTAVE_PERSISTANCE,
                DEFAULT_NOISE_OCTAVE_SCALE,
                true,
                temperatureSeed
            );

            // Hilliness computation
            float rawHill = ComputePerlin2D(
                static_cast<float>(globalX), static_cast<float>(globalY),
                HILLINESS_NOISE_SCALE,
                HILLINESS_NOISE_OCTAVES,
                DEFAULT_OCTAVE_PERSISTANCE,
                DEFAULT_NOISE_OCTAVE_SCALE,
                true,
                hillSeed
            );
            float hill = SmoothStep3(RangeMap(rawHill, -1.0f, 1.0f, 0.0f, 1.0f));

            // Ocean computation
            float ocean = ComputePerlin2D(
                static_cast<float>(globalX), static_cast<float>(globalY),
                OCEANESS_NOISE_SCALE,
                OCEANESS_NOISE_OCTAVES,
                DEFAULT_OCTAVE_PERSISTANCE,
                DEFAULT_NOISE_OCTAVE_SCALE,
                true,
                oceanSeed
            );

            // Raw terrain computation
            float rawTerrain = ComputePerlin2D(
                static_cast<float>(globalX), static_cast<float>(globalY),
                TERRAIN_NOISE_SCALE,
                TERRAIN_NOISE_OCTAVES,
                DEFAULT_OCTAVE_PERSISTANCE,
                DEFAULT_NOISE_OCTAVE_SCALE,
                true,
                terrainSeed
            );

            // Base terrain height with river/hill shaping
            float terrainHeightF = DEFAULT_TERRAIN_HEIGHT + hill * RangeMap(
                std::abs(rawTerrain), 0.0f, 1.0f, -RIVER_DEPTH, DEFAULT_TERRAIN_HEIGHT
            );

            // Ocean depressions
            if (ocean > OCEAN_START_THRESHOLD)
            {
                float oceanBlend = RangeMapClamped(ocean,
                                                   OCEAN_START_THRESHOLD,
                                                   OCEAN_END_THRESHOLD,
                                                   0.0f, 1.0f);
                terrainHeightF = terrainHeightF - Lerp(0.0f, OCEAN_DEPTH, oceanBlend);
            }

            // Dirt layer thickness driven by noise
            float dirtDepthPct = Get2dNoiseZeroToOne(globalX, globalY, dirtSeed);
            int   dirtDepth    = MIN_DIRT_OFFSET_Z + static_cast<int>(
                std::round(dirtDepthPct * (MAX_DIRT_OFFSET_Z - MIN_DIRT_OFFSET_Z))
            );

            int idxXY               = y * Chunk::CHUNK_SIZE_X + x;
            humidityMapXY[idxXY]    = humidity;
            temperatureMapXY[idxXY] = temperature;
            heightMapXY[idxXY]      = static_cast<int>(std::floor(terrainHeightF));
            dirtDepthXY[idxXY]      = dirtDepth;
        }
    }

    // --- Pass 2: assign block types for every (x,y,z) ---
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; ++z)
    {
        for (int y = 0; y < Chunk::CHUNK_SIZE_Y; ++y)
        {
            for (int x = 0; x < Chunk::CHUNK_SIZE_X; ++x)
            {
                IntVec3  localCoords(x, y, z);
                BlockPos globalBlockPos = chunk->LocalToWorld(x, y, z);
                IntVec3  globalCoords(globalBlockPos.x, globalBlockPos.y, globalBlockPos.z);
                size_t   blockIndex = Chunk::CoordsToIndex(x, y, z);
                UNUSED(blockIndex) // Use for debugging
                int idxXY = y * Chunk::CHUNK_SIZE_X + x;

                int   terrainHeight = heightMapXY[idxXY];
                int   dirtDepth     = dirtDepthXY[idxXY];
                float humidity      = humidityMapXY[idxXY];
                float temperature   = temperatureMapXY[idxXY];

                // Temperature-driven ice ceiling depth
                float iceDepth = DEFAULT_TERRAIN_HEIGHT - std::floor(RangeMapClamped(temperature,
                                                                                     ICE_TEMPERATURE_MAX,
                                                                                     ICE_TEMPERATURE_MIN,
                                                                                     ICE_DEPTH_MIN,
                                                                                     ICE_DEPTH_MAX));

                // Determine block type using optimized ID lookup
                int blockId = DetermineBlockTypeId(globalCoords, terrainHeight, dirtDepth,
                                                   humidity, temperature, iceDepth);

                // Set the block in the chunk using the cached block ID
                if (blockId >= 0)
                {
                    auto block = GetCachedBlockById(blockId);
                    if (block)
                    {
                        // Get default BlockState from Block (similar to Block.defaultBlockState() in Minecraft)
                        auto* blockState = block->GetDefaultState();
                        if (blockState)
                        {
                            chunk->SetBlock(x, y, z, blockState);
                        }
                    }
                    else
                    {
                        // Fallback to air if block not found
                        auto airBlock = GetCachedBlockById(m_airId);
                        if (airBlock)
                        {
                            auto* airBlockState = airBlock->GetDefaultState();
                            if (airBlockState)
                            {
                                chunk->SetBlock(x, y, z, airBlockState);
                            }
                        }
                    }
                }
            }
        }
    }

    // Mark chunk as generated and dirty for mesh building
    chunk->SetGenerated(true);
    chunk->MarkDirty();
    LogDebug("SimpleMinerGenerator", Stringf("Generated chunk (%d, %d) with SimpleMinerGenerator", chunkX, chunkZ));
}

std::shared_ptr<enigma::registry::block::Block> SimpleMinerGenerator::GetCachedBlock(const std::string& blockName) const
{
    // Check ID cache first
    auto idIt = m_blockIdCache.find(blockName);
    if (idIt != m_blockIdCache.end())
    {
        // Use O(1) numeric ID lookup
        return GetCachedBlockById(idIt->second);
    }

    // Cache miss - get from registry and cache both ID and block
    auto block = BlockRegistry::GetBlock("simpleminer", blockName);
    if (block)
    {
        int blockId = BlockRegistry::GetBlockId("simpleminer", blockName);
        if (blockId >= 0)
        {
            m_blockIdCache[blockName] = blockId;
            m_blockByIdCache[blockId] = block;
        }
    }
    return block;
}

std::shared_ptr<enigma::registry::block::Block> SimpleMinerGenerator::GetCachedBlockById(int blockId) const
{
    // Check block cache first
    auto blockIt = m_blockByIdCache.find(blockId);
    if (blockIt != m_blockByIdCache.end())
    {
        return blockIt->second;
    }

    // Cache miss - get from registry using O(1) lookup
    auto block = BlockRegistry::GetBlockById(blockId);
    if (block)
    {
        m_blockByIdCache[blockId] = block;
    }
    return block;
}

void SimpleMinerGenerator::InitializeBlockCache() const
{
    // Pre-cache all simpleminer namespace blocks
    auto allBlocks = BlockRegistry::GetBlocksByNamespace("simpleminer");
    for (const auto& block : allBlocks)
    {
        if (block)
        {
            int blockId = block->GetNumericId();
            if (blockId >= 0)
            {
                m_blockIdCache[block->GetRegistryName()] = blockId;
                m_blockByIdCache[blockId]                = block;
            }
        }
    }

    // Pre-cache commonly used block IDs for fast access during generation
    m_airId        = BlockRegistry::GetBlockId("simpleminer", "air");
    m_grassId      = BlockRegistry::GetBlockId("simpleminer", "grass");
    m_dirtId       = BlockRegistry::GetBlockId("simpleminer", "dirt");
    m_stoneId      = BlockRegistry::GetBlockId("simpleminer", "stone");
    m_sandId       = BlockRegistry::GetBlockId("simpleminer", "sand");
    m_waterId      = BlockRegistry::GetBlockId("simpleminer", "water");
    m_iceId        = BlockRegistry::GetBlockId("simpleminer", "ice");
    m_lavaId       = BlockRegistry::GetBlockId("simpleminer", "lava");
    m_obsidianId   = BlockRegistry::GetBlockId("simpleminer", "obsidian");
    m_coalOreId    = BlockRegistry::GetBlockId("simpleminer", "coal_ore");
    m_ironOreId    = BlockRegistry::GetBlockId("simpleminer", "iron_ore");
    m_goldOreId    = BlockRegistry::GetBlockId("simpleminer", "gold_ore");
    m_diamondOreId = BlockRegistry::GetBlockId("simpleminer", "diamond_ore");

    LogInfo("SimpleMinerGenerator", Stringf("Initialized block cache with %d blocks, pre-cached common block IDs",
                                            static_cast<int>(m_blockIdCache.size())));
}

std::string SimpleMinerGenerator::DetermineBlockType(const IntVec3& globalPos, int   terrainHeight,
                                                     int            dirtDepth, float humidity, float temperature,
                                                     float          iceDepth) const
{
    // Above terrain - air, water, or ice
    if (globalPos.z > terrainHeight)
    {
        if (globalPos.z < SEA_LEVEL_Z)
        {
            // Water and ice between surface and sea level
            if (temperature < 0.38f && globalPos.z > iceDepth)
            {
                return "ice";
            }
            return "water";
        }
        return "air";
    }

    // Surface block (grass vs sand by humidity and elevation)
    if (globalPos.z == terrainHeight)
    {
        if (humidity < MIN_SAND_HUMIDITY)
        {
            return "sand";
        }
        if (humidity < MAX_SAND_HUMIDITY && terrainHeight <= DEFAULT_TERRAIN_HEIGHT)
        {
            return "sand";
        }
        return "grass";
    }

    // Subsurface: dirt or sand cap above stone/ores
    int dirtTopZ = terrainHeight - dirtDepth;
    int sandTopZ = terrainHeight - static_cast<int>(std::floor(RangeMapClamped(humidity,
                                                                               MIN_SAND_DEPTH_HUMIDITY,
                                                                               MAX_SAND_DEPTH_HUMIDITY,
                                                                               SAND_DEPTH_MIN,
                                                                               SAND_DEPTH_MAX)));

    if (globalPos.z < terrainHeight && globalPos.z >= dirtTopZ)
    {
        if (globalPos.z >= sandTopZ)
        {
            return "sand";
        }
        return "dirt";
    }

    // Deep underground: special layers, lava/obsidian, ores, stone
    if (globalPos.z < dirtTopZ)
    {
        if (globalPos.z == OBSIDIAN_Z)
        {
            return "obsidian";
        }
        if (globalPos.z == LAVA_Z)
        {
            return "lava";
        }

        // Check for ores
        std::string oreType = DetermineOreType(globalPos);
        if (!oreType.empty())
        {
            return oreType;
        }

        return "stone";
    }

    // Default fallback
    return "air";
}

std::string SimpleMinerGenerator::DetermineOreType(const IntVec3& globalPos) const
{
    float oreNoise = Get3dNoiseZeroToOne(globalPos.x, globalPos.y, globalPos.z);

    if (oreNoise < DIAMOND_CHANCE)
    {
        return "diamond_ore";
    }
    if (oreNoise < GOLD_CHANCE)
    {
        return "gold_ore";
    }
    if (oreNoise < IRON_CHANCE)
    {
        return "iron_ore";
    }
    if (oreNoise < COAL_CHANCE)
    {
        return "coal_ore";
    }

    return ""; // No ore, will become stone
}

float SimpleMinerGenerator::ComputePerlin2D(float        x, float           y, float          scale, unsigned int octaves,
                                            float        persistence, float octaveScale, bool wrap,
                                            unsigned int seed) const
{
    UNUSED(wrap); // 传递给底层实现，但在此处未直接使用
    // Use the existing SmoothNoise implementation
    return Compute2dPerlinNoise(x, y, scale, octaves, persistence, octaveScale, true, seed);
}

float SimpleMinerGenerator::RangeMap(float value, float inMin, float inMax, float outMin, float outMax) const
{
    if (inMin == inMax) return outMin;
    float t = (value - inMin) / (inMax - inMin);
    return outMin + t * (outMax - outMin);
}

float SimpleMinerGenerator::RangeMapClamped(float value, float inMin, float inMax, float outMin, float outMax) const
{
    float t = (value - inMin) / (inMax - inMin);
    t       = std::clamp(t, 0.0f, 1.0f);
    return outMin + t * (outMax - outMin);
}

float SimpleMinerGenerator::Lerp(float a, float b, float t) const
{
    return a + t * (b - a);
}

float SimpleMinerGenerator::SmoothStep3(float t) const
{
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Optimized block type determination using pre-cached Block IDs
int SimpleMinerGenerator::DetermineBlockTypeId(const IntVec3& globalPos, int   terrainHeight,
                                               int            dirtDepth, float humidity, float temperature,
                                               float          iceDepth) const
{
    // Above terrain - air, water, or ice
    if (globalPos.z > terrainHeight)
    {
        if (globalPos.z < SEA_LEVEL_Z)
        {
            // Water and ice between surface and sea level
            if (temperature < 0.38f && globalPos.z > iceDepth)
            {
                return m_iceId;
            }
            return m_waterId;
        }
        return m_airId;
    }

    // Surface block (grass vs sand by humidity and elevation)
    if (globalPos.z == terrainHeight)
    {
        if (humidity < MIN_SAND_HUMIDITY)
        {
            return m_sandId;
        }
        if (humidity < MAX_SAND_HUMIDITY && terrainHeight <= DEFAULT_TERRAIN_HEIGHT)
        {
            return m_sandId;
        }
        return m_grassId;
    }

    // Subsurface: dirt or sand cap above stone/ores
    int dirtTopZ = terrainHeight - dirtDepth;
    int sandTopZ = terrainHeight - static_cast<int>(std::floor(RangeMapClamped(humidity,
                                                                               MIN_SAND_DEPTH_HUMIDITY,
                                                                               MAX_SAND_DEPTH_HUMIDITY,
                                                                               SAND_DEPTH_MIN,
                                                                               SAND_DEPTH_MAX)));

    if (globalPos.z < terrainHeight && globalPos.z >= dirtTopZ)
    {
        if (globalPos.z >= sandTopZ)
        {
            return m_sandId;
        }
        return m_dirtId;
    }

    // Deep underground: special layers, lava/obsidian, ores, stone
    if (globalPos.z < dirtTopZ)
    {
        if (globalPos.z == OBSIDIAN_Z)
        {
            return m_obsidianId;
        }
        if (globalPos.z == LAVA_Z)
        {
            return m_lavaId;
        }

        // Check for ores
        int oreId = DetermineOreTypeId(globalPos);
        if (oreId >= 0)
        {
            return oreId;
        }

        return m_stoneId;
    }

    // Default fallback
    return m_airId;
}

int SimpleMinerGenerator::DetermineOreTypeId(const IntVec3& globalPos) const
{
    float oreNoise = Get3dNoiseZeroToOne(globalPos.x, globalPos.y, globalPos.z);

    if (oreNoise < DIAMOND_CHANCE)
    {
        return m_diamondOreId;
    }
    if (oreNoise < GOLD_CHANCE)
    {
        return m_goldOreId;
    }
    if (oreNoise < IRON_CHANCE)
    {
        return m_ironOreId;
    }
    if (oreNoise < COAL_CHANCE)
    {
        return m_coalOreId;
    }

    return -1; // No ore, will become stone
}
