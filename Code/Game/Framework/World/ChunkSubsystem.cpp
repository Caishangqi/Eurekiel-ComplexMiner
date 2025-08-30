#include "ChunkSubsystem.hpp"

#include "World.hpp"
using namespace simpleminer::framework::world;

ChunkSubsystem::ChunkSubsystem(ChunkConfig& config) : m_config(config)
{
}

ChunkSubsystem::~ChunkSubsystem()
{
}

void ChunkSubsystem::Render() const
{
}

void ChunkSubsystem::Startup()
{
}

void ChunkSubsystem::Shutdown()
{
}

void ChunkSubsystem::Initialize()
{
    EngineSubsystem::Initialize();
}

bool ChunkSubsystem::RequiresInitialize() const
{
    return EngineSubsystem::RequiresInitialize();
}

void ChunkSubsystem::BeginFrame()
{
    EngineSubsystem::BeginFrame();
}

void ChunkSubsystem::Update(float deltaTime)
{
    EngineSubsystem::Update(deltaTime);
}

void ChunkSubsystem::EndFrame()
{
    EngineSubsystem::EndFrame();
}

std::shared_ptr<World> ChunkSubsystem::GetWorld(int id)
{
    for (const auto& world : m_worlds)
    {
        if (world->GetID() == id)
            return std::shared_ptr<World>(world.get(), [](World*){});  // 创建不删除的shared_ptr
    }
    return nullptr;
}

std::shared_ptr<World> ChunkSubsystem::GetWorld(const std::string& name)
{
    for (const auto& world : m_worlds)
    {
        if (world->GetName() == name)
            return std::shared_ptr<World>(world.get(), [](World*){});  // 创建不删除的shared_ptr
    }
    return nullptr;
}

std::shared_ptr<Chunk> ChunkSubsystem::GetChunk(int worldID, IntVec2 chunkCoords)
{
    // TODO: Implement chunk retrieval by world ID and coordinates
    return nullptr;
}

std::shared_ptr<Chunk> ChunkSubsystem::GetChunk(const std::string& worldName, IntVec2 chunkCoords)
{
    // TODO: Implement chunk retrieval by world name and coordinates
    return nullptr;
}

std::shared_ptr<Chunk> ChunkSubsystem::LoadChunk(int worldID, IntVec2 chunkCoords)
{
    // TODO: Implement chunk loading by world ID and coordinates
    return nullptr;
}

std::shared_ptr<Chunk> ChunkSubsystem::LoadChunk(const std::string& worldName, IntVec2 chunkCoords)
{
    // TODO: Implement chunk loading by world name and coordinates
    return nullptr;
}

void ChunkSubsystem::UnloadChunk(int worldID, IntVec2 chunkCoords)
{
}

void ChunkSubsystem::UnloadChunk(const std::string& worldName, IntVec2 chunkCoords)
{
}
