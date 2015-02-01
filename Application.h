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
#include "GUI.h"
#include "Mandelbrot.h"
#include "MandelbrotShader.h"
#include "Element2d.h"
#include "TextureShader.h"

#define INITIAL_MINIMAP_SIZE 256

//
// Wrapper class for terrain application.
//
class Application
{

    struct MandelMinimap
    {
        int width;
        int height;

        float xScale;
        float yScale;

        Vec2f upperLeft;
        Vec2f lowerRight;
        Vec2f poi;
        Vec2f poi2;

        int clickCnt;
    };

public:
    //  Constructor
    Application(void);

    //  Copy constructor
    Application(const Application &);

    //  Destructor
    ~Application(void);

    //  Initialization
    bool Initialize(HWND hwnd, int screenWidth, int screenHeight);

    //  Safely destroy graphics class.
    void Shutdown();

    // Process function called every frame.
    bool ProcessFrame();

    // Render the scene to screen.
    bool RenderGraphics();

    // Handle user input.
    bool HandleInput(float);

    // Helper to draw text as overlay
    bool drawText(ID3D11Device *pDevice,
                  ID3D11DeviceContext *pContext,
                  IDXGISwapChain *pSwapChain);

    // Getter methods
    bool IsFullScreen();
    bool IsVsync();
    float GetScreenDepth();
    float GetScreenNear();

    // Setter
    void SetLeftMouseDown(bool state);
    void SetRightMouseDown(bool state);

    // guarantee align 16 for heap allocations
    void *operator new(size_t size) throw()
    {
        return _aligned_malloc(size, 16);
    }

    void operator delete(void *ptr) throw()
    {
        _aligned_free(ptr);
    }

private:
    bool SetGuiParams();

    void HandleMinimapClicks(int mouseX, int mouseY,
                             bool isRightMouse, bool isLeftMouse);

    // Safe shutdown/delete handler
    template<typename T> void SafeShutdown(T *&obj)
    {
        if (obj)
        {
            obj->Shutdown();
            delete obj;
            obj = NULL;
        }
    }

    template<typename T> void SafeDelete(T *&obj)
    {
        if (obj)
        {
            delete obj;
            obj = NULL;
        }
    }

private:
    //  Member variables
    bool m_fullScreen;
    bool m_vSync;
    bool m_lockSurfaceCamera;
    bool m_stopAnimation;
    bool m_leftMouseDown;
    bool m_rightMouseDown;
    bool m_wireframe;
    bool m_backFaceCulling;

    // draw object bools
    bool m_drawSkyDome;
    bool m_drawOcean;
    bool m_drawTerrain;
    bool m_drawMandelbrot;
    // TODO callback methods
    bool m_oldDrawMandelbrot;
    bool m_oldDrawOcean;
    bool m_oldDrawTerrain;
    bool m_drawMinimap;

    // ocean settings
    int m_oceanTileFactor;
    float m_oceanTimeScale;
    float m_oceanHeightOffset;

    // terrain settings
    float m_terrainHurst;
    float m_terrainVariance;
    float m_terrainScaling;
    float m_terrainHeightScaling;
    int m_terrainResolution;
    // TODO: Callback methods
    float m_oldTerrainHurst;
    float m_oldTerrainVariance;
    float m_oldTerrainScaling;
    float m_oldTerrainHeightScaling;
    int m_oldTerrainResolution;

    // Mandelbrot settings
    bool m_mandelChanged;
    float m_mandelUpperLeftX;
    float m_mandelUpperLeftY;
    float m_mandelLowerRightX;
    float m_mandelLowerRightY;
    float m_mandelIterations;
    int m_mandelMaskSize;
    // TODO callbacks
    float m_oldMandelUpperLeftX;
    float m_oldMandelUpperLeftY;
    float m_oldMandelLowerRightX;
    float m_oldMandelLowerRightY;
    float m_oldMandelIterations;
    int m_oldMandelMaskSize;

    // Quad tree settings
    bool m_useQuadtree;
    bool m_oldUseQuadtree;
    int m_maxTrianglesQtNode;

    // Minimap settings struct
    MandelMinimap *m_pMandelMini;

    // Camera settings
    bool m_orbitalCamera;
    float m_zoom;
    float m_screenDepth;
    float m_screenNear;
    float m_spectatorHeight;
    float m_elapsedTime;

    // misc settings
    float m_guiLightDir[3];

    // window
    HWND m_hwnd;
    int m_screenWidth;
    int m_screenHeight;

    XMMATRIX m_baseViewMatrix;;

    // object pointer
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
    Frustum *m_pFrustum;
    QuadTree *m_pQuadTree;
    Font *m_pFont;
    Profiler *m_pProfiler;
    SkyDome *m_pSkyDome;
    SkyDomeShader *m_pSkyDomeShader;
    Ocean *m_pOcean;
    OceanShader *m_pOceanShader;
    GUI *m_pGUI;
    Mandelbrot *m_pMandelbrot;
    MandelbrotShader *m_pMandelbrotShader;
    Element2d *m_pMinimap;
    TextureShader *m_pMinimapShader;

    // textures
    Texture *m_pSkyDomeTex;
    vector<Texture *> m_vTerrainTextures;
};