#pragma once
#include "Game/Framework/GUISubsystem.hpp"

class BitmapFont;

class GUIGrid : public GUI
{
    DECLARE_GUI(GUIGrid, "GUIGrid", 100)

public:
    explicit GUIGrid();
    ~GUIGrid() override;

    void Draw() override;
    void Update(float deltaTime) override;
    void OnCreate() override;
    void OnDestroy() override;

private:
    BitmapFont* m_font = nullptr;
};
