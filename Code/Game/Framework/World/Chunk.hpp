#pragma once
namespace simpleminer::framework::world
{
    class Chunk
    {
    public:
        Chunk();
        ~Chunk();

        void Render() const;
        void Update();
    };
}
