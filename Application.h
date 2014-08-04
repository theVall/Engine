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
#include "Ocean.h"
#include "OceanShader.h"

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
    bool m_stopAnimation;

    float m_screenDepth;
    float m_screenNear;
    float m_spectatorHeight;
    float m_elapsedTime;

    HWND m_hwnd;

    Camera *m_pCamera;
    Input *m_pInput;
    D3D *m_pDirect3D;
    Terrain *m_pTerrain;
    ColorShader *m_pColorShader;
    Position *m_pPosition;
    Timer *m_pTimer;
    Util *m_pUtil;
    TerrainShader *m_pTerrainShader;
    Light *m_pLight;
    Texture *m_pGroundTex;
    Texture *m_pSkyDomeTex;
    Frustum *m_pFrustum;
    QuadTree *m_pQuadTree;
    Font *m_pFont;
    Profiler *m_pProfiler;
    SkyDome *m_pSkyDome;
    SkyDomeShader *m_pSkyDomeShader;
    Ocean *m_pOcean;
    OceanShader *m_pOceanShader;

    ID3D11RenderTargetView *pNullRTV;
};