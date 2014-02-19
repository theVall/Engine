#pragma once

#include "Input.h"
#include "D3D.h"
#include "Camera.h"
#include "Terrain.h"
#include "ColorShader.h"
#include "Position.h"
#include "Timer.h"
#include "ImageUtil.h"
#include "Frustum.h"
#include "QuadTree.h"
//#include "Fps.h"
//#include "Cpu.h"
//#include "FontShader.h"
//#include "Text.h"

#include "TerrainShader.h"
#include "Light.h"
#include "Texture.h"

//
// Wrapper class for terrain application.
//
class Application
{
public:
    //  Constructor
    Application(void);

    //  Copy constructor
    Application(const Application &);

    //  Destructor
    ~Application(void);

    //  Initialization
    bool Initialize(HWND, int, int);

    //  Safely destroy graphics class.
    void Shutdown();

    // Process function called every frame.
    bool ProcessFrame();

    // Render the scene to screen.
    bool RenderGraphics();

    // Handle user input.
    bool HandleInput(float);

    //  Getter methods
    bool IsFullScreen();
    bool IsVsync();
    float GetScreenDepth();
    float GetScreenNear();

private:
    //  Member variables
    bool m_fullScreen;
    bool m_vSync;
    float m_screenDepth;
    float m_screenNear;

    HWND m_hwnd;

    D3D *m_D3D;
    Camera* m_Camera;
    Input* m_Input;
    D3D* m_Direct3D;
    Terrain* m_Terrain;
    ColorShader* m_ColorShader;
    Position* m_Position;
    Timer* m_Timer;
    ImageUtil* m_ImageUtil;
    TerrainShader* m_TerrainShader;
    Light* m_Light;
    Texture* m_Texture;
    Frustum* m_Frustum;
    QuadTree* m_QuadTree;
    //Model* m_Model;
    //TextureShader* m_TextureShader;
    //Element2d* m_Element2d;
};