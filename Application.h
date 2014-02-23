#pragma once

#include <sstream>

#include "Input.h"
#include "D3D.h"
#include "Camera.h"
#include "Terrain.h"
#include "ColorShader.h"
#include "Position.h"
#include "Timer.h"
#include "Util.h"
#include "Frustum.h"
#include "QuadTree.h"
#include "TerrainShader.h"
#include "Light.h"
#include "Texture.h"
#include "Font.h"
#include "Profiler.h"
#include "SkyDome.h"
#include "SkyDomeShader.h"

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

    bool drawText(ID3D11Device *, ID3D11DeviceContext *, IDXGISwapChain *);

    //  Getter methods
    bool IsFullScreen();
    bool IsVsync();
    float GetScreenDepth();
    float GetScreenNear();

private:
    //  Member variables
    bool m_fullScreen;
    bool m_vSync;
    bool m_lockSurfaceCamera;

    float m_screenDepth;
    float m_screenNear;
    float m_SpectatorHeight;

    HWND m_hwnd;

    D3D *m_D3D;
    Camera* m_Camera;
    Input* m_Input;
    D3D* m_Direct3D;
    Terrain* m_Terrain;
    ColorShader* m_ColorShader;
    Position* m_Position;
    Timer* m_Timer;
    Util* m_Util;
    TerrainShader* m_TerrainShader;
    Light* m_Light;
    Texture* m_Texture;
    Frustum* m_Frustum;
    QuadTree* m_QuadTree;
    Font* m_Font;
    Profiler* m_Profiler;
    SkyDome* m_SkyDome;
    SkyDomeShader* m_SkyDomeShader;
};