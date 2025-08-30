#include "Entity.hpp"

#include "Engine/Core/EngineCommon.hpp"

Entity::Entity(Game* owner): m_game(owner)
{
}

Entity::~Entity()
{
    m_game = nullptr;
}

void Entity::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds)
}

Mat44 Entity::GetModelToWorldTransform() const
{
    // Mat44::MakeNonUniformScale3D(m_scale);
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    matTranslation.Append(Mat44::MakeNonUniformScale3D(m_scale));
    return matTranslation;
}
