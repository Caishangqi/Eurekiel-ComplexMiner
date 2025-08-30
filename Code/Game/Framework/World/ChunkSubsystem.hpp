#pragma once
#include "Engine/Core/SubsystemManager.hpp"
#include "Engine/Math/IntVec2.hpp"

namespace simpleminer::framework::world
{
    class Chunk;
    class World;

    struct ChunkConfig
    {
    };

    using namespace enigma::core;

    class ChunkSubsystem : public EngineSubsystem
    {
    public:
        DECLARE_SUBSYSTEM(ChunkSubsystem, "chunk", 100)

        explicit ChunkSubsystem(ChunkConfig& config);
        ~ChunkSubsystem() override;

        void Render() const;

        void Startup() override;
        void Shutdown() override;
        void Initialize() override;
        bool RequiresInitialize() const override;
        void BeginFrame() override;
        void Update(float deltaTime) override;
        void EndFrame() override;

        /// Helper functions
        std::shared_ptr<World> GetWorld(int id);
        std::shared_ptr<World> GetWorld(const std::string& name);
        std::shared_ptr<Chunk> GetChunk(int worldID, IntVec2 chunkCoords);
        std::shared_ptr<Chunk> GetChunk(const std::string& worldName, IntVec2 chunkCoords);
        std::shared_ptr<Chunk> LoadChunk(int worldID, IntVec2 chunkCoords);
        std::shared_ptr<Chunk> LoadChunk(const std::string& worldName, IntVec2 chunkCoords);
        void                   UnloadChunk(int worldID, IntVec2 chunkCoords);
        void                   UnloadChunk(const std::string& worldName, IntVec2 chunkCoords);

    private:
        ChunkConfig&                        m_config;
        std::vector<std::unique_ptr<World>> m_worlds;
    };
}
