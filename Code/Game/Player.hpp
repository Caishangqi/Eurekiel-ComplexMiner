#pragma once
#include "Entity.hpp"

class Camera;

class Player : public Entity
{
public:
    Player(Game* owner);
    ~Player() override;

    Camera* m_camera = nullptr;

    void Update(float deltaSeconds) override;
    void Render() const override;
};
