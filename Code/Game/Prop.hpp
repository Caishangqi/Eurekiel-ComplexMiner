#pragma once
#include <vector>

#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"

class Prop : public Entity
{
public:
    Prop(Game* game);
    ~Prop() override;

    void Update(float deltaSeconds) override;
    void Render() const override;

    std::vector<Vertex_PCU> m_vertexes;
    Rgba8                   m_color   = Rgba8::WHITE;
    Texture*                m_texture = nullptr;
};
