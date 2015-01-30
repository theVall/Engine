#include "Application.h"

#define DEBUG

Application::Application()
{
    // initial properties - TODO: load/save from file
    m_fullScreen        = false;
    m_vSync             = false;
    m_lockSurfaceCamera = false;
    m_stopAnimation     = true;
    m_leftMouseDown     = false;
    m_rightMouseDown    = false;
    m_wireframe         = false;
    m_backFaceCulling   = true;

    m_drawSkyDome       = true;
    m_drawOcean         = m_oldDrawOcean        = false;
    m_drawTerrain       = m_oldDrawTerrain      = true;
    m_drawMandelbrot    = m_oldDrawMandelbrot   = false;
    m_drawMinimap       = true;

    // terrain settings
    m_terrainHurst          = m_oldTerrainHurst         = 0.75f;
    m_terrainVariance       = m_oldTerrainVariance      = 1.0f;
    m_terrainScaling        = m_oldTerrainScaling       = 14.0f;
    m_terrainHeightScaling  = m_oldTerrainHeightScaling = 20.0f;
    m_terrainResolution     = m_oldTerrainResolution    = 8;

    // Quad tree
    m_useQuadtree        = m_oldUseQuadtree = true;
    m_maxTrianglesQtNode = 5000;

    // Mandelbrot settings
    m_mandelChanged     = false;
    m_mandelUpperLeftX  = m_oldMandelUpperLeftX     = -2.1f;
    m_mandelUpperLeftY  = m_oldMandelUpperLeftY     = 1.2f;
    m_mandelLowerRightX = m_oldMandelLowerRightX    = 0.6f;
    m_mandelLowerRightY = m_oldMandelLowerRightY    = -1.2f;
    m_mandelIterations  = m_oldMandelIterations     = 500.0f;
    m_mandelMaskSize    = m_oldMandelMaskSize       = 9;

    // ocean settings
    m_oceanTileFactor   = 7;
    m_oceanTimeScale    = 0.0003f;
    m_oceanHeightOffset = -m_terrainHeightScaling - 5.0f;

    // camera settings
    m_orbitalCamera     = false;
    m_zoom              = 1.0f;
    m_screenDepth       = 10000.0f;
    m_screenNear        = 0.1f;
    m_spectatorHeight   = 10.0f;
    m_elapsedTime       = 0;

    // set pointer to null
    m_pInput            = nullptr;
    m_pDirect3D         = nullptr;
    m_pCamera           = nullptr;
    m_pTerrain          = nullptr;
    m_pColorShader      = nullptr;
    m_pTimer            = nullptr;
    m_pPosition         = nullptr;
    m_pUtil             = nullptr;
    m_pTerrainShader    = nullptr;
    m_pLight            = nullptr;
    m_pSkyDomeTex       = nullptr;
    m_pFrustum          = nullptr;
    m_pQuadTree         = nullptr;
    m_pFont             = nullptr;
    m_pProfiler         = nullptr;
    m_pSkyDome          = nullptr;
    m_pSkyDomeShader    = nullptr;
    m_pOcean            = nullptr;
    m_pOceanShader      = nullptr;
    m_pGUI              = nullptr;
    m_pMandelbrot       = nullptr;
    m_pMandelbrotShader = nullptr;
    m_pMinimap          = nullptr;
    m_pMinimapShader    = nullptr;
}


Application::Application(const Application &)
{
}


Application::~Application()
{
}


bool Application::Initialize(HWND hwnd, int screenWidth, int screenHeight)
{
    bool result;

// TODO: advanced profiling output
//    char videoCard[128];
//    int videoMemory;

    // set window parameters
    m_hwnd = hwnd;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Create and initialize __input__ object for keyboard and mouse handling.
    m_pInput = new Input;
    if (!m_pInput)
    {
        return false;
    }
    result = m_pInput->Initialize(m_hwnd, m_screenWidth, m_screenHeight);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize _D3D_ window object.
    m_pDirect3D = new D3D();
    if (!m_pDirect3D)
    {
        return false;
    }
    result = m_pDirect3D->Initialize(screenWidth,
                                     screenHeight,
                                     m_vSync,
                                     m_hwnd,
                                     m_fullScreen,
                                     m_screenDepth,
                                     m_screenNear);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __camera__ object.
    m_pCamera = new Camera();
    if (!m_pCamera)
    {
        return false;
    }
    // Initialize a base view matrix with the camera for 2D user interface rendering.
    m_pCamera->SetPosition(Vec3f(0.0f, 0.0f, -1.0f));
    m_pCamera->Render();
    m_pCamera->GetViewMatrix(m_baseViewMatrix);
    // Set the initial position of the camera.
    Vec3f camPos;
    camPos.x = 100.0f * m_terrainScaling;
    camPos.y = 20.0f * m_terrainHeightScaling;
    camPos.z = 100.0f * m_terrainScaling;
    m_pCamera->SetPosition(camPos);

    // image loading utility object
    m_pUtil = new Util;
    if (!m_pUtil)
    {
        return false;
    }

    // Create and initialize the __terrain__ object.
    m_pTerrain = new Terrain;
    if (!m_pTerrain)
    {
        return false;
    }
    result = m_pTerrain->GenerateDiamondSquare(m_pUtil,
                                               m_terrainResolution,
                                               m_terrainHurst,
                                               m_terrainVariance,
                                               m_terrainScaling,
                                               m_terrainHeightScaling);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize __shader__.
    m_pTerrainShader = new TerrainShader;
    if (!m_pTerrainShader)
    {
        return false;
    }
    result = m_pTerrainShader->Initialize(m_pDirect3D->GetDevice(),
                                          m_hwnd,
                                          L"../Engine/shader/TerrainVS.hlsl",
                                          L"../Engine/shader/TerrainPS.hlsl");
    if (!result)
    {
        MessageBox(m_hwnd,
                   L"Could not initialize the terrain shader object.",
                   L"Error",
                   MB_OK);
        return false;
    }

    // Create and initialize __light__.
    m_pLight = new Light;
    if (!m_pLight)
    {
        return false;
    }
    m_pLight->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
    m_pLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_pLight->SetDirection(0.40f, -0.30f, 0.15f);
    // light vector in GUI
    m_guiLightDir[0] = m_pLight->GetDirection().x;
    m_guiLightDir[1] = m_pLight->GetDirection().y;
    m_guiLightDir[2] = m_pLight->GetDirection().z;

    // Create the __frustum__ for view frustum culling
    m_pFrustum = new Frustum;
    if (!m_pFrustum)
    {
        return false;
    }

    // Create and initialize the __font wrapper__ object.
    m_pFont = new Font;
    if (!m_pFont)
    {
        MessageBox(m_hwnd, L"Could not font wrapper object.", L"Error", MB_OK);
        return false;
    }
    m_pFont->Initialize(L"Arial", m_pDirect3D->GetDevice());

    m_pDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      L"VallEngine",
                      50.0f,
                      m_screenWidth / 2.0f,
                      m_screenHeight / 2.0f - 25.0f,
                      0xff00ffff, // yellow
                      0x1 | 0x4); // center text
    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      L"Initializing...",
                      25.0f,
                      m_screenWidth / 2.0f,
                      m_screenHeight / 2.0f + 50.0f,
                      0xff00ffff, // yellow
                      0x1 | 0x4); // center text
    m_pDirect3D->EndScene();

    // Create and initialize the __quad tree__.
    m_pQuadTree = new QuadTree;
    if (!m_pQuadTree)
    {
        return false;
    }
    result = m_pQuadTree->Initialize(m_pTerrain,
                                     m_pDirect3D->GetDevice(),
                                     m_maxTrianglesQtNode,
                                     m_useQuadtree);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize __textures__.
    m_pSkyDomeTex = new Texture;
    if (!m_pSkyDomeTex)
    {
        return false;
    }
    if (!m_pSkyDomeTex->LoadFromDDS(m_pDirect3D->GetDevice(),
                                    L"../Engine/res/tex/sky2_new.dds"))
    {
        MessageBox(m_hwnd, L"Error loading sky dome texture.", L"Error", MB_OK);
        return false;
    }
    // terrain textures
    vector<WCHAR *> terrainTexFilenames;
    terrainTexFilenames.push_back(L"../Engine/res/tex/sand.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/rock.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/mossyRock.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/grass.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/snow.dds");

    for (size_t i = 0; i < terrainTexFilenames.size(); ++i)
    {
        m_vTerrainTextures.push_back(new Texture);
        if (!m_vTerrainTextures.back()->LoadFromDDS(m_pDirect3D->GetDevice(),
                                                    terrainTexFilenames.at(i)))
        {
            MessageBox(m_hwnd, L"Error loading terrain texture.", L"Error", MB_OK);
            return false;
        }
    }

    // Create and initialize the __timer__ object.
    m_pTimer = new Timer;
    if (!m_pTimer)
    {
        return false;
    }
    result = m_pTimer->Initialize();
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __position__ object.
    m_pPosition = new Position;
    if (!m_pPosition)
    {
        MessageBox(m_hwnd, L"Could not initialize the position object.", L"Error", MB_OK);
        return false;
    }
    // Set the initial position of the viewer to the same as the initial camera position.
    m_pPosition->SetPosition(camPos);

    // Create and initialize the __profiler__ object.
    m_pProfiler = new Profiler;
    if (!m_pProfiler)
    {
        MessageBox(m_hwnd, L"Could not initialize the profiling object.", L"Error", MB_OK);
        return false;
    }
    m_pProfiler->Initialize();

    // Create and initialize the __sky dome__ object.
    m_pSkyDome = new SkyDome;
    if (!m_pSkyDome)
    {
        return false;
    }
    if (!m_pSkyDome->Initialize(m_pDirect3D->GetDevice(),
                                L"../Engine/res/model/dome.txt",
                                m_pUtil))
    {
        MessageBox(m_hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __sky dome shader__ object.
    m_pSkyDomeShader = new SkyDomeShader;
    if (!m_pSkyDomeShader)
    {
        return false;
    }
    if (!m_pSkyDomeShader->Initialize(m_pDirect3D->GetDevice(),
                                      m_hwnd,
                                      L"../Engine/shader/SkyDomeVS.hlsl",
                                      L"../Engine/shader/SkyDomePS.hlsl"))
    {
        MessageBox(m_hwnd,
                   L"Could not initialize the sky dome shader object.",
                   L"Error",
                   MB_OK);
        return false;
    }

    // Create and initialize the __Ocean__ object
    m_pOcean = new Ocean;
    if (!m_pOcean)
    {
        return false;
    }
    Ocean::OceanParameter oceanParams = { 512,                  // map dim
                                          m_oceanTimeScale,     // time factor
                                          0.3f,                 // amplitude
                                          Vec2f(0.3f, 0.6f),    // wind direction
                                          400.0f,               // wind speed
                                          0.07f,                // wind dependency
                                          1.3f                  // choppy
                                        };
    if (!m_pOcean->Initialize(oceanParams,
                              m_pDirect3D->GetDevice(),
                              m_pDirect3D->GetDeviceContext(),
                              m_hwnd,
                              L"../Engine/shader/OceanSimVS.hlsl",
                              L"../Engine/shader/OceanSimPS.hlsl",
                              L"../Engine/shader/OceanSimCS.hlsl"))
    {
        MessageBox(m_hwnd, L"Could not initialize the ocean object.", L"Error", MB_OK);
        return false;
    }
    // generate ocean height map for the first time
    m_pOcean->UpdateDisplacement(0.0f, m_pDirect3D->GetDeviceContext());

    // Create and initialize the __ocean shader__ object.
    m_pOceanShader = new OceanShader(oceanParams.displacementMapDim);
    if (!m_pOceanShader)
    {
        return false;
    }
    if (!m_pOceanShader->Initialize(m_pDirect3D->GetDevice(),
                                    m_hwnd,
                                    L"../Engine/shader/OceanVS.hlsl",
                                    L"../Engine/shader/OceanPS.hlsl"))
    {
        MessageBox(m_hwnd, L"Could not initialize the ocean shader object.", L"Error", MB_OK);
        return false;
    }
    m_pOceanShader->SetTileCount(m_oceanTileFactor);

    // Create the __GUI__ for text output and GUI elements
    m_pGUI = new GUI;
    if (!m_pGUI)
    {
        return false;
    }
    if (!m_pGUI->Initialize(m_pDirect3D->GetDevice(),
                            "Settings",
                            m_screenWidth,
                            m_screenHeight))
    {
        MessageBox(hwnd, L"Could not initialize the GUI object.", L"Error", MB_OK);
        return false;
    }

    // Create GUI variables
    if (!SetGuiParams())
    {
        MessageBox(hwnd, L"Could not initialize all GUI parameters.", L"Error", MB_OK);
        return false;
    }

    //  Create and initialize the __Mandelbrot__ object
    m_pMandelbrot = new Mandelbrot;
    if (!m_pMandelbrot)
    {
        return false;
    }
    if (!m_pMandelbrot->Initialize(m_pDirect3D->GetDevice(),
                                   m_pDirect3D->GetDeviceContext(),
                                   m_hwnd,
                                   L"../Engine/shader/MandelbrotCS.hlsl",
                                   1 << m_terrainResolution))
    {
        MessageBox(m_hwnd, L"Could not initialize the Mandelbrot object.", L"Error", MB_OK);
        return false;
    }

    m_pMandelbrot->CalcHeightsInRectangle(Vec2f(m_mandelUpperLeftX, m_mandelUpperLeftY),
                                          Vec2f(m_mandelLowerRightX, m_mandelLowerRightY),
                                          m_mandelIterations,
                                          m_terrainVariance,
                                          m_mandelMaskSize,
                                          m_pDirect3D->GetDeviceContext());

    //  Create and initialize the __Mandelbrot Shader__ program
    m_pMandelbrotShader = new MandelbrotShader(1 << m_terrainResolution);
    if (!m_pMandelbrotShader)
    {
        return false;
    }
    if (!m_pMandelbrotShader->Initialize(m_pDirect3D->GetDevice(),
                                         m_hwnd,
                                         L"../Engine/shader/MandelbrotVS.hlsl",
                                         L"../Engine/shader/MandelbrotPS.hlsl"))
    {
        MessageBox(m_hwnd, L"Could not initialize the Mandelbrot shader object.", L"Error", MB_OK);
        return false;
    }

    //  Create and initialize the __Minimap__ object.
    m_pMinimap = new Element2d;
    if (!m_pMinimap)
    {
        return false;
    }
    // Mandelbrot Minimap settings struct
    m_pMandelMini = new MandelMinimap;
    m_pMandelMini->clickCnt = 0;
    m_pMandelMini->upperLeft = Vec2f(m_mandelUpperLeftX, m_mandelUpperLeftY);
    m_pMandelMini->lowerRight = Vec2f(m_mandelLowerRightX, m_mandelLowerRightY);
    m_pMandelMini->xScale = fabs(m_mandelUpperLeftX - m_mandelLowerRightX);
    m_pMandelMini->yScale = fabs(m_mandelUpperLeftY - m_mandelLowerRightY);
    m_pMandelMini->width = (int)(INITIAL_MINIMAP_SIZE / 2.0f * m_pMandelMini->xScale);
    m_pMandelMini->height = (int)(INITIAL_MINIMAP_SIZE / 2.0f * m_pMandelMini->yScale);

    if (!m_pMinimap->Initialize(m_pDirect3D->GetDevice(),
                                m_screenWidth,
                                m_screenHeight,
                                m_pMandelMini->width,
                                m_pMandelMini->height,
                                m_pMandelbrot->GetHeightMap()))
    {
        MessageBox(m_hwnd, L"Could not initialize the Minimap object.", L"Error", MB_OK);
        return false;
    }
    //  Create and initialize the __Minimap Shader__ object.
    m_pMinimapShader = new TextureShader;
    if (!m_pMinimapShader)
    {
        return false;
    }
    if (!m_pMinimapShader->Initialize(m_pDirect3D->GetDevice(),
                                      m_hwnd,
                                      L"../Engine/shader/TextureVS.hlsl",
                                      L"../Engine/shader/MinimapPS.hlsl"))
    {
        MessageBox(m_hwnd, L"Could not initialize the Minimap Shader object.", L"Error", MB_OK);
        return false;
    }


    return true;
}


void Application::Shutdown()
{
    SafeShutdown(m_pSkyDomeShader);
    SafeShutdown(m_pSkyDome);
    SafeShutdown(m_pOceanShader);
    SafeShutdown(m_pOcean);
    SafeShutdown(m_pFont);
    SafeShutdown(m_pQuadTree);
    SafeShutdown(m_pTerrainShader);
    SafeShutdown(m_pTerrain);
    SafeShutdown(m_pDirect3D);
    SafeShutdown(m_pInput);
    SafeShutdown(m_pGUI);
    SafeShutdown(m_pMandelbrot);
    SafeShutdown(m_pMandelbrotShader);
    //SafeShutdown(m_pMinimap);
    // TODO: error on SRV release
    SafeShutdown(m_pMinimapShader);

    SafeDelete(m_pProfiler);
    SafeDelete(m_pPosition);
    SafeDelete(m_pTimer);
    SafeDelete(m_pFrustum);
    SafeDelete(m_pLight);
    SafeDelete(m_pUtil);
    SafeDelete(m_pCamera);
    SafeDelete(m_pMandelMini);

    SafeDelete(m_pSkyDomeTex);
    for (size_t i = 0; i < m_vTerrainTextures.size(); ++i)
    {
        SafeDelete(m_vTerrainTextures.at(i));

    }

    return;
}

bool Application::ProcessFrame()
{
    float height;

    // Update the system stats.
    m_pTimer->Frame();
    m_pProfiler->Frame();

    m_elapsedTime += m_pTimer->GetTime();

    // Do the frame input processing.
    if (!HandleInput(m_pTimer->GetTime()))
    {
        return false;
    }

    if (m_lockSurfaceCamera && m_drawTerrain)
    {
        Vec3f position = m_pCamera->GetPosition();
        // Get the height of the triangle that is directly underneath the camera position.
        // If there is a triangle under the camera position,
        // set the camera above it, according to the spectator height.
        if (m_pQuadTree->GetHeightAtPosition(position.x, position.z, height))
        {
            Vec3f newPos = Vec3f(position.x, height + m_spectatorHeight, position.z);
            m_pCamera->SetPosition(newPos);
        }
    }

    // Ocean displacement
    if (m_drawOcean && !m_stopAnimation)
    {
        m_pOcean->UpdateDisplacement(m_elapsedTime, m_pDirect3D->GetDeviceContext());
    }

    if (m_drawMandelbrot && m_mandelChanged)
    {
        // On resolution change: create new Mandelbrot and -shader objects
        if (m_oldTerrainResolution != m_terrainResolution)
        {
            // Shutdown old Mandelbrot objects
            m_pMandelbrotShader->Shutdown();
            m_pMandelbrot->Shutdown();

            // Create the new objects
            if (!m_pMandelbrot->Initialize(m_pDirect3D->GetDevice(),
                                           m_pDirect3D->GetDeviceContext(),
                                           m_hwnd,
                                           L"../Engine/shader/MandelbrotCS.hlsl",
                                           1 << m_terrainResolution))
            {
                MessageBox(m_hwnd, L"Could not initialize the Mandelbrot object.", L"Error", MB_OK);
                return false;
            }

            m_pMandelbrotShader = new MandelbrotShader(1 << m_terrainResolution);
            if (!m_pMandelbrotShader)
            {
                return false;
            }
            if (!m_pMandelbrotShader->Initialize(m_pDirect3D->GetDevice(),
                                                 m_hwnd,
                                                 L"../Engine/shader/MandelbrotVS.hlsl",
                                                 L"../Engine/shader/MandelbrotPS.hlsl"))
            {
                MessageBox(m_hwnd, L"Could not initialize the Mandelbrot shader object.", L"Error", MB_OK);
                return false;
            }

            // Update resolution memory variable.
            m_oldTerrainResolution = m_terrainResolution;
        }

        m_pMandelbrot->CalcHeightsInRectangle(Vec2f(m_mandelUpperLeftX, m_mandelUpperLeftY),
                                              Vec2f(m_mandelLowerRightX, m_mandelLowerRightY),
                                              m_mandelIterations,
                                              m_terrainVariance,
                                              m_mandelMaskSize,
                                              m_pDirect3D->GetDeviceContext());

        // update minimap scaling
        m_pMandelMini->xScale = fabs(m_mandelUpperLeftX - m_mandelLowerRightX);
        m_pMandelMini->yScale = fabs(m_mandelUpperLeftY - m_mandelLowerRightY);
        m_pMandelMini->width = (int)(INITIAL_MINIMAP_SIZE / 2.0f * m_pMandelMini->xScale);
        m_pMandelMini->height = (int)(INITIAL_MINIMAP_SIZE / 2.0f * m_pMandelMini->yScale);

        if (m_pMandelMini->width*(1.0f + 0.1f*m_terrainScaling) < INITIAL_MINIMAP_SIZE &&
                m_pMandelMini->height*(1.0f + 0.1f*m_terrainScaling) < INITIAL_MINIMAP_SIZE)
        {
            m_pMandelMini->width *= (int)m_terrainScaling;
            m_pMandelMini->height *= (int)m_terrainScaling;
        }

        m_pMinimap->SetElementWidth(m_pMandelMini->width);
        m_pMinimap->SetElementHeight(m_pMandelMini->height);

        m_mandelChanged = false;
    }

    // Render the graphics.
    if (!RenderGraphics())
    {
        return false;
    }

    return true;
}


bool Application::HandleInput(float frameTime)
{
    short keyDown = false;
    bool moveCamOnDrag = true;

    int mouseX = 0;
    int mouseY = 0;

    // basic sensitivity, higher is faster
    float sensitivity = 0.25f;

    // Set the frame time for calculating the updated position.
    m_pPosition->SetFrameTime(frameTime);

    // Handle the __keyboard__ input.
    keyDown = GetAsyncKeyState(VK_ESCAPE);
    if (keyDown)
    {
        return false;
    }
    keyDown = GetAsyncKeyState(VK_SHIFT);
    if (keyDown)
    {
        sensitivity = 0.75f;
    }

    if (!m_orbitalCamera)
    {
        keyDown = GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W');
        m_pPosition->MoveForward((keyDown == 1) || (keyDown == 0x8000), sensitivity);

        keyDown = GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A');
        m_pPosition->MoveLeft((keyDown == 1) || (keyDown == 0x8000), sensitivity);

        keyDown = GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S');
        m_pPosition->MoveBackward((keyDown == 1) || (keyDown == 0x8000), sensitivity);

        keyDown = GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D');
        m_pPosition->MoveRight((keyDown == 1) || (keyDown == 0x8000), sensitivity);

        keyDown = GetAsyncKeyState(VK_SPACE);
        m_pPosition->MoveUpward(keyDown != 0, sensitivity);

        keyDown = GetAsyncKeyState('C');
        m_pPosition->MoveDownward(keyDown != 0, sensitivity);
    }
    else
    {
        // allow walking mode only with standard camera mode
        // (GUI showing workaround) TODO: disable in GUI
        m_lockSurfaceCamera = false;
    }

    keyDown = GetAsyncKeyState('R');
    // Terrain mode: generate new random terrain.
    if (keyDown != 0 && m_drawTerrain)
    {
        m_pTerrain->GenNewRand();
        m_terrainHurst += 0.000001f;
    }
    // Mandelbrot mode: reset terrain.
    else if (keyDown != 0 && m_drawMandelbrot)
    {
        m_mandelChanged = true;
        // Reset Mandelbrot area coordinates
        m_mandelUpperLeftX = -2.1f;
        m_mandelUpperLeftY = 1.2f;
        m_mandelLowerRightX = 0.6f;
        m_mandelLowerRightY = -1.2f;
        // Reset terrain scaling parameters
        m_terrainHeightScaling = 20;
        m_terrainScaling = 1;
        // Reset minimap
        m_pMandelMini->width = INITIAL_MINIMAP_SIZE;
        m_pMandelMini->height = INITIAL_MINIMAP_SIZE;
    }

    // Yaw and pitch with __mouse__ movement.
    if (moveCamOnDrag)
    {
        if (m_leftMouseDown)
        {
            m_pInput->GetMouseLocationChange(mouseX, mouseY);

            // select first minimap point with left mouse click
            if (m_drawMandelbrot && m_drawMinimap)
            {
                HandleMinimapClicks(mouseX, mouseY, false, true);
            }

            m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.5f);
        }
        else if (m_rightMouseDown)
        {
            m_pInput->GetMouseLocationChange(mouseX, mouseY);

            // select second minimap point with right mouse click
            if (m_drawMandelbrot && m_drawMinimap)
            {
                HandleMinimapClicks(mouseX, mouseY, true, false);
            }

            // zoom on right mouse button down
            if (m_orbitalCamera)
            {
                int mouseXLoc;
                int mouseYLoc;
                m_pInput->GetMouseLocation(mouseXLoc, mouseYLoc);

                if (mouseY > 0)
                {
                    m_zoom += (mouseY + mouseYLoc) / (m_pProfiler->GetFps() / 10.0f);
                }
                else if (mouseY < 0)
                {
                    m_zoom -= (-mouseY + mouseYLoc) / (m_pProfiler->GetFps() / 10.0f);
                }
            }
        }
        else
        {
            m_pInput->GetMouseLocationChange(mouseX, mouseY);

            // select second minimap point with right mouse click
            if (m_drawMandelbrot && m_drawMinimap)
            {
                HandleMinimapClicks(mouseX, mouseY, false, false);
            }
        }
    }
    else
    {
        m_pInput->GetMouseLocationChange(mouseX, mouseY);
        m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.5f);
    }


    Vec3f pos;
    Vec3f rot;

    // Get the view point position/rotation.
    m_pPosition->GetPosition(pos);
    m_pPosition->GetRotation(rot);

    // Set the position of the camera.
    m_pCamera->SetPosition(pos);
    m_pCamera->SetRotation(rot);

    // Handle GUI parameters
    m_pDirect3D->SetFullscreen(m_fullScreen);
    m_pDirect3D->SetWireframe(m_wireframe);
    // workaround. TODO: fix wireframe ocean animation
    if (m_wireframe)
    {
        m_stopAnimation = true;
    }
    m_pOceanShader->SetTileCount(m_oceanTileFactor);
    m_pOcean->SetTimeScale(m_oceanTimeScale);
    m_pLight->SetDirection(m_guiLightDir[0], m_guiLightDir[1], m_guiLightDir[2]);

    // Only bother with rebuilding terrain if it is actually drawn...
    if (m_drawTerrain)
    {
        // workaround, TODO: use callback methods...
        if (m_oldTerrainHurst != m_terrainHurst ||
                m_oldTerrainVariance != m_terrainVariance ||
                m_oldTerrainResolution != m_terrainResolution ||
                m_oldTerrainScaling != m_terrainScaling ||
                m_oldTerrainHeightScaling != m_terrainHeightScaling ||
                m_oldUseQuadtree != m_useQuadtree ||
                m_oldDrawTerrain != m_drawTerrain)
        {
            m_pQuadTree->Shutdown();
            m_pTerrain->Shutdown();

            if (!m_pTerrain->GenerateDiamondSquare(m_pUtil,
                                                   m_terrainResolution,
                                                   m_terrainHurst,
                                                   m_terrainVariance,
                                                   m_terrainScaling,
                                                   m_terrainHeightScaling))
            {
                MessageBox(m_hwnd,
                           L"Something went wrong while generating terrain.",
                           L"Error",
                           MB_OK);
                return false;
            }

            // rebuild quad-tree
            m_pQuadTree->Initialize(m_pTerrain,
                                    m_pDirect3D->GetDevice(),
                                    m_maxTrianglesQtNode,
                                    m_useQuadtree);

            // update memory values
            m_oldTerrainHurst = m_terrainHurst;
            m_oldTerrainVariance = m_terrainVariance;
            m_oldTerrainScaling = m_terrainScaling;
            m_oldTerrainHeightScaling = m_terrainHeightScaling;
            m_oldTerrainResolution = m_terrainResolution;
            m_oldUseQuadtree = m_useQuadtree;
        }
    }
    else if (m_drawMandelbrot)
    {
        // Check if Mandelbrot terrain parameters have changed.
        // If so, set flag for recalculating Mandelbrot rectangle.
        if (m_oldTerrainResolution != m_terrainResolution ||
                m_oldTerrainVariance != m_terrainVariance ||
                m_oldTerrainScaling != m_terrainScaling ||
                m_oldMandelUpperLeftX != m_mandelUpperLeftX ||
                m_oldMandelUpperLeftY != m_mandelUpperLeftY ||
                m_oldMandelLowerRightX != m_mandelLowerRightX ||
                m_oldMandelLowerRightY != m_mandelLowerRightY ||
                m_oldMandelIterations != m_mandelIterations ||
                m_oldMandelMaskSize != m_mandelMaskSize ||
                m_oldDrawMandelbrot != m_drawMandelbrot)
        {
            m_mandelChanged = true;

            // Update memory variables.
            // Resolution is updated in process frame because it is needed for a check.
            m_oldTerrainVariance = m_terrainVariance;
            m_oldTerrainScaling = m_terrainScaling;
            m_oldMandelUpperLeftX = m_mandelUpperLeftX;
            m_oldMandelUpperLeftY = m_mandelUpperLeftY;
            m_oldMandelLowerRightX = m_mandelLowerRightX;
            m_oldMandelLowerRightY = m_mandelLowerRightY;
            m_oldMandelIterations = m_mandelIterations;
            m_oldMandelMaskSize = m_mandelMaskSize;
        }
    }

    if (m_drawMandelbrot != m_oldDrawMandelbrot ||
            m_drawOcean != m_oldDrawOcean ||
            m_drawTerrain != m_oldDrawTerrain)
    {
        // hide terrain and ocean in Mandelbrot mode
        if ((m_drawMandelbrot != m_oldDrawMandelbrot) && m_drawMandelbrot)
        {
            m_terrainResolution = 10;
            m_terrainHeightScaling = 20;
            m_terrainScaling = 1;
            m_terrainVariance = 5.0f;

            m_drawTerrain = false;
            m_drawOcean = false;
        }
        else if ((m_drawMandelbrot != m_oldDrawMandelbrot) && !m_drawMandelbrot)
        {
            m_terrainResolution = 8;
            m_terrainHeightScaling = 20;
            m_terrainScaling = 14;
            m_terrainVariance = 1.0f;

            m_drawTerrain = true;
            m_drawOcean = true;
            m_stopAnimation = false;
        }

        // Adjust GUI according to visible objects.
        if (!SetGuiParams())
        {
            return false;
        }

        m_oldDrawMandelbrot = m_drawMandelbrot;
        m_oldDrawOcean = m_drawOcean;
        m_oldDrawTerrain = m_drawTerrain;
    }

    return true;
}


bool Application::RenderGraphics()
{
    XMMATRIX worldMatrix;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX orthoMatrix;
    Vec3f cameraPosition;

    // Clear the scene.
    m_pDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position and mode.
    if (m_orbitalCamera)
    {
        // Calculate target point from terrain size for orbital cam.
        float target = (float)(1 << m_terrainResolution) * (float)m_terrainScaling / 2.0f;
        if (m_drawTerrain)
        {
            m_pCamera->RenderOrbital(Vec3f(target, 0.0f, target), m_zoom);
        }
        else if (m_drawMandelbrot)
        {
            // Care: Mandelbrot terrain is not square. Use scaling factors from minimap.
            m_pCamera->RenderOrbital(Vec3f(target * m_pMandelMini->xScale,
                                           0.0f,
                                           target * m_pMandelMini->yScale),
                                     m_zoom);
        }

    }
    else
    {
        m_pCamera->Render();
    }

    m_pDirect3D->GetWorldMatrix(worldMatrix);
    m_pCamera->GetViewMatrix(viewMatrix);
    m_pDirect3D->GetProjectionMatrix(projectionMatrix);
    m_pDirect3D->GetOrthoMatrix(orthoMatrix);

    // Sky dome calculations
    cameraPosition = m_pCamera->GetPosition();
    worldMatrix = XMMatrixTranslation(cameraPosition.x,
                                      cameraPosition.y - 0.25f,
                                      cameraPosition.z);

    // Render the sky dome.
    if (m_drawSkyDome)
    {
        m_pDirect3D->TurnOffCulling();
        m_pDirect3D->TurnZBufferOff();

        m_pSkyDome->Render(m_pDirect3D->GetDeviceContext());
        m_pSkyDomeShader->Render(m_pDirect3D->GetDeviceContext(),
                                 m_pSkyDome->GetIndexCount(),
                                 worldMatrix,
                                 viewMatrix,
                                 projectionMatrix,
                                 m_pSkyDome->GetApexColor(),
                                 m_pSkyDome->GetCenterColor(),
                                 m_pSkyDomeTex->GetSrv());
    }
    m_pDirect3D->TurnZBufferOn();
    if (m_backFaceCulling)
    {
        m_pDirect3D->TurnOnCulling();
    }
    else
    {
        m_pDirect3D->TurnOffCulling();
    }

    // Reset the world matrix.
    m_pDirect3D->GetWorldMatrix(worldMatrix);

    // construct view frustum
    m_pFrustum->ConstructFrustum(projectionMatrix, viewMatrix, m_screenDepth);

    // Render the terrain geometry
    if (m_drawTerrain)
    {
        if (!m_pTerrainShader->SetShaderParameters(m_pDirect3D->GetDeviceContext(),
                                                   worldMatrix,
                                                   viewMatrix,
                                                   projectionMatrix,
                                                   m_pLight->GetDirection(),
                                                   m_pLight->GetAmbientColor(),
                                                   m_pLight->GetDiffuseColor(),
                                                   m_vTerrainTextures,
                                                   m_terrainHeightScaling))
        {
            return false;
        }

        // Render the terrain using the quad tree and terrain shader.
        m_pQuadTree->Render(m_pFrustum,
                            m_pDirect3D->GetDeviceContext(),
                            m_pTerrainShader,
                            m_wireframe);
    }

    if (m_drawMandelbrot)
    {
        m_pMandelbrotShader->Render(m_pDirect3D->GetDeviceContext(),
                                    worldMatrix,
                                    viewMatrix,
                                    projectionMatrix,
                                    m_pLight->GetDirection(),
                                    m_pMandelbrot->GetHeightMap(),
                                    m_wireframe,
                                    Vec2f(m_mandelUpperLeftX, m_mandelUpperLeftY),
                                    Vec2f(m_mandelLowerRightX, m_mandelLowerRightY),
                                    m_terrainScaling,
                                    m_terrainHeightScaling);
    }

    // Render the ocean geometry
    if (m_drawOcean)
    {
        // enable variable sea level (GUI parameter)
        worldMatrix = XMMatrixTranslation(0.0f, m_oceanHeightOffset, 0.0f);

        m_pOceanShader->Render(m_pDirect3D->GetDeviceContext(),
                               worldMatrix,
                               viewMatrix,
                               projectionMatrix,
                               cameraPosition.GetAsXMFloat3(),
                               m_pLight->GetDirection(),
                               m_pOcean->GetDisplacementMap(),
                               m_pOcean->GetGradientMap(),
                               m_pSkyDomeTex->GetSrv(),
                               m_wireframe);
    }

// profiling/debug output
#ifdef DEBUG
    int fps = m_pProfiler->GetFps();

    std::wostringstream fpswchar;
    fpswchar << fps << " FPS";

    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      (WCHAR *)fpswchar.str().c_str(),
                      16.0f,
                      1150.0f,
                      20.0f,
                      0xff8cc63e,
                      0);

    std::wostringstream triangleswchar;
    triangleswchar << m_pQuadTree->GetDrawCount() << " Tris";
    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      (WCHAR *)triangleswchar.str().c_str(),
                      16.0f,
                      1150.0f,
                      40.0f,
                      0xff8cc63e,
                      0);

    std::wostringstream randSeed;
    randSeed << m_pTerrain->GetRand() << " Seed";
    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      (WCHAR *)randSeed.str().c_str(),
                      16.0f,
                      1150.0f,
                      60.0f,
                      0xff8cc63e,
                      0);
#endif

    // gender AntTweakBar
    m_pGUI->RenderGUI();

    if (m_drawMandelbrot && m_drawMinimap)
    {
        // create minimap render object
        m_pMinimap->Render(m_pDirect3D->GetDeviceContext(),
                           m_screenWidth - m_pMandelMini->width,
                           m_screenHeight - m_pMandelMini->height);
        // render minimap
        m_pMinimapShader->Render(m_pDirect3D->GetDeviceContext(),
                                 2 * 3, // two triangles
                                 worldMatrix,
                                 m_baseViewMatrix,
                                 orthoMatrix,
                                 m_pMandelbrot->GetHeightTex(),
                                 (float)m_pMandelMini->width,
                                 (float)m_pMandelMini->height,
                                 (float)(1 << m_terrainResolution),
                                 (float)(1 << m_terrainResolution),
                                 m_pMandelMini->poi,
                                 m_pMandelMini->poi2);
    }

    // Present the rendered scene to the screen.
    m_pDirect3D->EndScene();

    return true;
}


bool Application::IsFullScreen()
{
    return m_fullScreen;
}


bool Application::IsVsync()
{
    return m_vSync;
}


float Application::GetScreenDepth()
{
    return m_screenDepth;
}


float Application::GetScreenNear()
{
    return m_screenNear;
}


void Application::SetLeftMouseDown(bool state)
{
    m_leftMouseDown = state;

    if (state)
    {
        m_pInput->GetMousePoint();
    }
}


void Application::SetRightMouseDown(bool state)
{
    m_rightMouseDown = state;
}



bool Application::SetGuiParams()
{
    m_pGUI->ClearAll();

    // not working yet, re-initialization of D3D required?
    //m_pGUI->AddBoolVar("vSync", m_vSync);

    // TODO: Does not make sense as long as window is not re-sizable
    //if (!m_pGUI->AddBoolVar("FullScreen", m_fullScreen))
    //{
    //    return false;
    //}

    // Standard settings
    if (!m_pGUI->AddBoolVar("Wireframe", m_wireframe, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("Back-face Culling", m_backFaceCulling, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("Orbital Camera", m_orbitalCamera, ""))
    {
        return false;
    }
    if (m_drawTerrain)
    {
        if (!m_pGUI->AddBoolVar("Walking Mode", m_lockSurfaceCamera, ""))
        {
            return false;
        }
    }

    if (!m_pGUI->AddSeperator(NULL, NULL))
    {
        return false;
    }

    // Render options
    if (!m_pGUI->AddBoolVar("Mandelbrot Mode", m_drawMandelbrot, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("Render Sky", m_drawSkyDome, ""))
    {
        return false;
    }

    if (!m_drawMandelbrot)
    {
        if (!m_pGUI->AddBoolVar("Render Ocean", m_drawOcean, ""))
        {
            return false;
        }
        if (!m_pGUI->AddBoolVar("Render Terrain", m_drawTerrain, ""))
        {
            return false;
        }
    }
    else
    {
        if (!m_pGUI->AddBoolVar("Render Minimap", m_drawMinimap, ""))
        {
            return false;
        }
    }


    // Mandelbrot pixel game settings
    if (m_drawMandelbrot)
    {
        if (!m_pGUI->AddFloatVar("Iterations",
                                 m_mandelIterations,
                                 "min=500.0 max=100000.0 step=500 group='Pixel Game Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Upper Left x",
                                 m_mandelUpperLeftX,
                                 "min=-2.1 max=0.6 step=0.01 precision=5 group='Pixel Game Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Upper Left y",
                                 m_mandelUpperLeftY,
                                 "min=-1.2 max=1.2 step=0.01 precision=5 group='Pixel Game Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Lower Right x",
                                 m_mandelLowerRightX,
                                 "min=-2.1 max=0.6 step=0.01 precision=5 group='Pixel Game Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Lower Right y",
                                 m_mandelLowerRightY,
                                 "min=-1.2 max=1.2 step=0.01 precision=5 group='Pixel Game Settings'"))
        {
            return false;
        }
    }

    // Terrain settings
    if (m_drawTerrain || m_drawMandelbrot)
    {
        if (m_drawMandelbrot)
        {
            if (!m_pGUI->AddIntVar("Resolution",
                                   m_terrainResolution,
                                   "min=2 max=12 step=1 group='Terrain Settings'"))
            {
                return false;
            }
        }
        else
        {
            if (!m_pGUI->AddIntVar("Resolution",
                                   m_terrainResolution,
                                   "min=2 max=10 step=1 group='Terrain Settings'"))
            {
                return false;
            }
        }
        if (!m_pGUI->AddFloatVar("Scaling",
                                 m_terrainScaling,
                                 "min=1.0 max=100.0 step=1 group='Terrain Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Height Scaling",
                                 m_terrainHeightScaling,
                                 "min=1.0 max=100.0 step=1 group='Terrain Settings'"))
        {
            return false;
        }
        if (m_drawMandelbrot)
        {
            if (!m_pGUI->AddIntVar("Gauss Mask Size",
                                   m_mandelMaskSize,
                                   "min=1 max=15 step=2 group='Terrain Settings'"))
            {
                return false;
            }
        }
        if (!m_pGUI->AddFloatVar("Variance",
                                 m_terrainVariance,
                                 "min=0 max=10.0 step=0.01 group='Terrain Settings'"))
        {
            return false;
        }
    }
    if (!m_drawMandelbrot)
    {
        if (!m_pGUI->AddFloatVar("Hurst Operator",
                                 m_terrainHurst,
                                 "min=0 max=1.0 step=0.01 group='Terrain Settings'"))
        {
            return false;
        }

        // Quad tree settings
        if (!m_pGUI->AddBoolVar("Use Quad-Tree", m_useQuadtree, "group='Quad Tree Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddIntVar("Max Triangles",
                               m_maxTrianglesQtNode,
                               "min=1000 max=500000 step=10000 group='Quad Tree Settings'"))
        {
            return false;
        }
    }

    // Ocean Settings
    if (m_drawOcean)
    {
        if (!m_pGUI->AddBoolVar("Pause Animation",
                                m_stopAnimation,
                                "group='Ocean Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddIntVar("Tile Factor",
                               m_oceanTileFactor,
                               "min=1 max=10 step=1 group='Ocean Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Animation Speed",
                                 m_oceanTimeScale,
                                 "min=0 max=0.005 step=0.00001 group='Ocean Settings'"))
        {
            return false;
        }
        if (!m_pGUI->AddFloatVar("Sea Level",
                                 m_oceanHeightOffset,
                                 "min=-250 max=250 step=1 group='Ocean Settings'"))
        {
            return false;
        }
    }

    // Lighting settings
    if (!m_pGUI->AddVec3fVar("Light Direction", m_guiLightDir[0], ""))
    {
        return false;
    }

    // Credits
    if (!m_pGUI->AddSeperator(NULL, NULL))
    {
        return false;
    }
    if (!m_pGUI->AddLabel(" label='(c) 2015 - Valentin Bruder' "))
    {
        return false;
    }

    return true;
}


void Application::HandleMinimapClicks(int mouseX, int mouseY,
                                      bool isRightMouse, bool isLeftMouse)
{
    // check if clicked inside the minimap
    m_pInput->GetMouseLocation(mouseX, mouseY);
    int minimapLeft = m_screenWidth - m_pMandelMini->width;
    int minimapTop = m_screenHeight - m_pMandelMini->height;

    if (mouseX > minimapLeft && mouseY > minimapTop)
    {
        // Map mouse coordinates to minimap coordinate system.
        Vec2f mouseMinimapCoords = Vec2f((float)mouseX - minimapLeft,
                                         (float)mouseY - minimapTop);
        // normalize to 0..1
        mouseMinimapCoords.x /= (float)m_pMandelMini->width;
        mouseMinimapCoords.y /= (float)m_pMandelMini->height;

        if (!isRightMouse && isLeftMouse)
        {
            m_pMandelMini->poi = mouseMinimapCoords;
        }
        else if (!isRightMouse && !isLeftMouse)
        {
            m_pMandelMini->poi2 = mouseMinimapCoords;
            return;
        }

        // map mouse minimap coordinates to current displayed
        // coordinate system
        mouseMinimapCoords.x *= m_pMandelMini->xScale;
        mouseMinimapCoords.y *= m_pMandelMini->yScale;
        mouseMinimapCoords += Vec2f(m_mandelUpperLeftX, m_mandelLowerRightY);
        // sign correction
        mouseMinimapCoords.y *= -1;

        // avoid recognizing the same point more than once
        if (m_pMandelMini->upperLeft != mouseMinimapCoords &&
                m_pMandelMini->lowerRight != mouseMinimapCoords)
        {
            // if first selected point
            if (m_pMandelMini->clickCnt == 0 && !isRightMouse)
            {
                m_pMandelMini->upperLeft = mouseMinimapCoords;
                m_pMandelMini->clickCnt++;
            }
            else if (m_pMandelMini->clickCnt == 1 &&
                     isRightMouse &&
                     ((m_pMandelMini->upperLeft.x < mouseMinimapCoords.x &&
                       m_pMandelMini->upperLeft.y > mouseMinimapCoords.y) ||
                      (m_pMandelMini->upperLeft.x > mouseMinimapCoords.x &&
                       m_pMandelMini->upperLeft.y < mouseMinimapCoords.y)))
            {
                m_pMandelMini->lowerRight = mouseMinimapCoords;
                // match the points depending on position
                if (m_pMandelMini->lowerRight.x < m_pMandelMini->upperLeft.x)
                {
                    Vec2f tmp = m_pMandelMini->lowerRight;
                    m_pMandelMini->lowerRight = m_pMandelMini->upperLeft;
                    m_pMandelMini->upperLeft = tmp;
                }

                // set new border points
                m_mandelUpperLeftX = m_pMandelMini->upperLeft.x;
                m_mandelUpperLeftY = m_pMandelMini->upperLeft.y;
                m_mandelLowerRightX = m_pMandelMini->lowerRight.x;
                m_mandelLowerRightY = m_pMandelMini->lowerRight.y;
                // clear POI
                m_pMandelMini->clickCnt = 0;
                m_pMandelMini->poi = Vec2f(-10.0f);
            }
        }
    }
    // clear POI
    m_pMandelMini->poi2 = Vec2f(-10.0f);
}
