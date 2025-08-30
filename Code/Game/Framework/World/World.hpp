#pragma once
#include <memory>
#include <string>
#include <vector>

namespace simpleminer::framework::world
{
    class Chunk;
}


namespace simpleminer::framework::world
{
    class World
    {
    public:
        explicit World(int id, std::string name);
        ~World();

        void Render() const;
        void Update();

        /// Getter
        int                GetID() const { return m_id; }
        const std::string& GetName() const { return m_name; }

    private:
        int         m_id   = 0;
        std::string m_name = "world";

        std::vector<std::unique_ptr<Chunk>> m_activeChunks;
        std::vector<std::unique_ptr<Chunk>> m_inactiveChunks;
        std::vector<std::unique_ptr<Chunk>> m_pendingChunks;
    };
}
