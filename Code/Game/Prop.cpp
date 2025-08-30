#include "Prop.hpp"

#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

Prop::Prop(Game* game) : Entity(game)
{
}

Prop::~Prop()
{
}

void Prop::Update(float deltaSeconds)
{
    Entity::Update(deltaSeconds);
    m_orientation = m_orientation + m_angularVelocity * deltaSeconds;
}

void Prop::Render() const
{
    g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_color);
    //g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    //g_theRenderer->SetBlendMode(BlendMode::ALPHA);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->DrawVertexArray(m_vertexes);
}
