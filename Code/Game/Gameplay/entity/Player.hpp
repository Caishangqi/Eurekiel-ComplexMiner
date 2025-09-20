#pragma once
#include "Entity.hpp"
#include "Engine/Core/EventSystem.hpp"

class GUIBlock3DSelection;
class GUIPlayerInventory;
class Camera;

class Player : public Entity
{
public:
    static bool Event_Player_Join_World(EventArgs& args);
    static bool Event_Player_Quit_World(EventArgs& args);

public:
    Player(Game* owner);
    ~Player() override;

    Camera* m_camera = nullptr;

    void Update(float deltaSeconds) override;
    void Render() const override;

    void ProcessInput(float deltaTime);

private:
    static std::shared_ptr<GUIPlayerInventory>  m_guiPlayerInventory;
    static std::shared_ptr<GUIBlock3DSelection> m_guiBlockSelection;
};
