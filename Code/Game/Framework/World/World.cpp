#include "World.hpp"

#include "Chunk.hpp"
using namespace simpleminer::framework::world;

World::World(int id, std::string name) : m_id(id), m_name(name)
{
}

World::~World()
{
}

void World::Render() const
{
    for (auto& m_active_chunk : m_activeChunks)
    {
        m_active_chunk->Render();
    }
}

void World::Update()
{
    for (auto& m_active_chunk : m_activeChunks)
    {
        m_active_chunk->Update();
    }
}
