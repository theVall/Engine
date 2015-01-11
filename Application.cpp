#include "Application.h"

#define DEBUG

Application::Application()
{
    // initial properties - TODO: load/save from file
    m_fullScreen        = false;
    m_vSync             = false;
    m_lockSurfaceCamera = false;
    m_stopAnimation     = false;
    m_leftMouseDown     = false;
    m_rightMouseDown    = false;
    m_wireframe         = false;

    m_drawSkyDome       = true;
    m_drawOcean         = false;
    m_drawTerrain       = true;

    // terrain settings
    m_terrainHurst          = m_oldTerrainHurst         = 0.5f;
    m_terrainVariance       = m_oldTerrainVariance      = 1.0f;
    m_terrainScaling        = m_oldTerrainScaling       = 14.0f;
    m_terrainHeightScaling  = m_oldTerrainHeightScaling = 10.0f;
    m_terrainResolution     = m_oldTerrainResolution    = 8;

    m_useQuadtree        = false;
    m_maxTrianglesQtNode = 5000;

    // ocean settings
    m_oceanTileFactor   = 7;
    m_oceanTimeScale    = 0.0003f;
    m_oceanHeightOffset = -m_terrainScaling;
    // camera settings
    m_orbitalCamera     = false;
    m_zoom              = 1.0f;
    m_screenDepth       = 5000.0f;
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

    float cameraX;
    float cameraY;
    float cameraZ;

    XMMATRIX baseViewMatrix;
//    char videoCard[128];
//    int videoMemory;

    m_hwnd = hwnd;

    // Create and initialize __input__ object for keyboard and mouse handling.
    m_pInput = new Input;
    if (!m_pInput)
    {
        return false;
    }
    result = m_pInput->Initialize(m_hwnd, screenWidth, screenHeight);
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
    m_pCamera->SetPosition(0.0f, 0.0f, -1.0f);
    m_pCamera->Render();
    m_pCamera->GetViewMatrix(baseViewMatrix);
    // Set the initial position of the camera.
    cameraX = 100.0f;
    cameraY = 200.0f;
    cameraZ = 100.0f;
    m_pCamera->SetPosition(cameraX, cameraY, cameraZ);

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
        MessageBox(m_hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
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
    m_pLight->SetDirection(0.2f, -0.33f, 0.3f);

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
                      L"Loading...\n",
                      20.0f,
                      50.0f,
                      50.0f,
                      0xff8cc63e,
                      0);
    m_pDirect3D->EndScene();

    // Create and initialize the __quad tree__.
    m_pQuadTree = new QuadTree;
    if (!m_pQuadTree)
    {
        return false;
    }
    result = m_pQuadTree->Initialize(m_pTerrain,
                                     m_pDirect3D->GetDevice(),
                                     m_maxTrianglesQtNode);
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
    if (!m_pSkyDomeTex->LoadFromDDS(m_pDirect3D->GetDevice(), L"../Engine/res/tex/sky2.dds"))
    {
        MessageBox(m_hwnd, L"Error loading sand texture.", L"Error", MB_OK);
        return false;
    }
    // terrain textures
    vector<WCHAR *> terrainTexFilenames;
    terrainTexFilenames.push_back(L"../Engine/res/tex/sand.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/rock.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/mossyRock.dds");
    terrainTexFilenames.push_back(L"../Engine/res/tex/grass.dds");

    for (int i = 0; i < 4; ++i)
    {
        m_vTerrainTextures.push_back(new Texture);
        if (!m_vTerrainTextures.back()->LoadFromDDS(m_pDirect3D->GetDevice(), terrainTexFilenames.at(i)))
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
    m_pPosition->SetPosition(cameraX, cameraY, cameraZ);

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
    if (!m_pSkyDome->Initialize(m_pDirect3D->GetDevice(), L"../Engine/res/model/dome.txt", m_pUtil))
    {
        MessageBox(hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __sky dome shader__ object.
    m_pSkyDomeShader = new SkyDomeShader;
    if (!m_pSkyDomeShader)
    {
        return false;
    }
    if (!m_pSkyDomeShader->Initialize(m_pDirect3D->GetDevice(),
                                      hwnd,
                                      L"../Engine/shader/SkyDomeVS.hlsl",
                                      L"../Engine/shader/SkyDomePS.hlsl"))
    {
        MessageBox(hwnd, L"Could not initialize the sky dome shader object.", L"Error", MB_OK);
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
                                          0.2f,                 // amplitude
                                          Vec2f(0.3f, 0.6f),    // wind direction
                                          400.0f,               // wind speed
                                          0.07f,                // wind dependency
                                          1.3f                  // choppy
                                        };
    if (!m_pOcean->Initialize(oceanParams,
                              m_pDirect3D->GetDevice(),
                              m_pDirect3D->GetDeviceContext(),
                              hwnd,
                              L"../Engine/shader/OceanSimVS.hlsl",
                              L"../Engine/shader/OceanSimPS.hlsl",
                              L"../Engine/shader/OceanSimCS.hlsl"))
    {
        MessageBox(hwnd, L"Could not initialize the ocean object.", L"Error", MB_OK);
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
                                    hwnd,
                                    L"../Engine/shader/OceanVS.hlsl",
                                    L"../Engine/shader/OceanPS.hlsl"))
    {
        MessageBox(hwnd, L"Could not initialize the ocean shader object.", L"Error", MB_OK);
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
                            screenWidth,
                            screenHeight))
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

    SafeDelete(m_pProfiler);
    SafeDelete(m_pPosition);
    SafeDelete(m_pTimer);
    SafeDelete(m_pFrustum);
    SafeDelete(m_pLight);
    SafeDelete(m_pUtil);
    SafeDelete(m_pCamera);

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

    if (m_lockSurfaceCamera)
    {
        Vec3f position = m_pCamera->GetPosition();
        // Get the height of the triangle that is directly underneath the camera position.
        // If there was a triangle under the camera position,
        // set the camera two units above it.
        if (m_pQuadTree->GetHeightAtPosition(position.x, position.z, height))
        {
            m_pCamera->SetPosition(position.x, height + m_spectatorHeight, position.z);
        }
    }

    // Ocean displacement
    if (!m_stopAnimation)
    {
        m_pOcean->UpdateDisplacement(m_elapsedTime, m_pDirect3D->GetDeviceContext());
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

    float posX;
    float posY;
    float posZ;

    float rotX;
    float rotY;
    float rotZ;

    float sensitivity = 0.1f;

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
        sensitivity = 0.5f;
    }

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

    // Yaw and pitch with __mouse__ movement.
    if (moveCamOnDrag)
    {
        if (m_leftMouseDown)
        {
            m_pInput->GetMouseLocationChange(mouseX, mouseY);
            m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.5f);
        }
    }
    else
    {
        m_pInput->GetMouseLocationChange(mouseX, mouseY);
        m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.5f);
    }

    // zoom on right mouse button down
    if (m_rightMouseDown && m_orbitalCamera)
    {
        int mouseXLoc;
        int mouseYLoc;

        m_pInput->GetMouseLocation(mouseXLoc, mouseYLoc);
        m_pInput->GetMouseLocationChange(mouseX, mouseY);
        if (mouseY > 0)
        {
            m_zoom += (mouseY + mouseYLoc) / (m_pProfiler->GetFps() / 6);
        }
        else if (mouseY < 0)
        {
            m_zoom -= (-mouseY + mouseYLoc) / (m_pProfiler->GetFps() / 6);
        }
    }

    // Get the view point position/rotation.
    m_pPosition->GetPosition(posX, posY, posZ);
    m_pPosition->GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_pCamera->SetPosition(posX, posY, posZ);
    m_pCamera->SetRotation(rotX, rotY, rotZ);

    // Handle GUI parameters
    m_pDirect3D->SetFullscreen(m_fullScreen);
    m_pDirect3D->SetWireframe(m_wireframe);
    m_pOceanShader->SetTileCount(m_oceanTileFactor);
    m_pOcean->SetTimeScale(m_oceanTimeScale);

    if (!m_useQuadtree)
    {
        m_maxTrianglesQtNode = 1000000;
    }

    // workaround, TODO: use callback methods...
    if (m_oldTerrainHurst != m_terrainHurst ||
            m_oldTerrainVariance != m_terrainVariance ||
            m_oldTerrainResolution != m_terrainResolution ||
            m_oldTerrainScaling != m_terrainScaling ||
            m_oldTerrainHeightScaling != m_terrainHeightScaling)
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
            MessageBox(m_hwnd, L"Something went wrong while generating the terrain.", L"Error", MB_OK);
            return false;
        }

        // rebuild quadtree
        m_pQuadTree->Initialize(m_pTerrain, m_pDirect3D->GetDevice(), m_maxTrianglesQtNode);

        // update memory values
        m_oldTerrainHurst = m_terrainHurst;
        m_oldTerrainVariance = m_terrainVariance;
        m_oldTerrainScaling = m_terrainScaling;
        m_oldTerrainHeightScaling = m_terrainHeightScaling;
        m_oldTerrainResolution = m_terrainResolution;
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

    // Generate the view matrix based on the camera's position.
    if (m_orbitalCamera)
    {
        // Calculate target point from terrain size
        float target = (float)m_pTerrain->GetWidth() * (float)m_terrainScaling / 2.0f;
        m_pCamera->RenderOrbital(Vec3f(target, 0.0f, target), m_zoom);
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
    worldMatrix = XMMatrixTranslation(cameraPosition.x, cameraPosition.y - 0.25f, cameraPosition.z);

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
    m_pDirect3D->TurnOnCulling();

    // Reset the world matrix.
    m_pDirect3D->GetWorldMatrix(worldMatrix);

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
                                                   m_terrainScaling * m_terrainHeightScaling))
        {
            return false;
        }

        // Render the terrain using the quad tree and terrain shader.
        m_pQuadTree->Render(m_pFrustum,
                            m_pDirect3D->GetDeviceContext(),
                            m_pTerrainShader,
                            m_wireframe);
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
#endif

    m_pGUI->RenderGUI();

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
    // not working yet, re-initialization of D3D required?
    //m_pGUI->AddBoolVar("vSync", m_vSync);

    // TODO: Does not make sense as long as window is not re-sizable
    //if (!m_pGUI->AddBoolVar("FullScreen", m_fullScreen))
    //{
    //    return false;
    //}

    if (!m_pGUI->AddBoolVar("Pause", m_stopAnimation, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("Wireframe", m_wireframe, ""))
    {
        return false;
    }

    if (!m_pGUI->AddBoolVar("WalkingMode", m_lockSurfaceCamera, ""))
    {
        return false;
    }

    if (!m_pGUI->AddBoolVar("OrbitalCamera", m_orbitalCamera, ""))
    {
        return false;
    }

    if (!m_pGUI->AddSeperator(NULL, NULL))
    {
        return false;
    }

    if (!m_pGUI->AddBoolVar("RenderSky", m_drawSkyDome, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("RenderOcean", m_drawOcean, ""))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("RenderTerrain", m_drawTerrain, ""))
    {
        return false;
    }

    // Terrain Settings
    if (!m_pGUI->AddIntVar("Resolution",
                           m_terrainResolution,
                           "min=2 max=10 step=1 group='TerrainSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("Scaling",
                             m_terrainScaling,
                             "min=1.0 max=20.0 step=1 group='TerrainSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("HeightScaling",
                             m_terrainHeightScaling,
                             "min=1.0 max=20.0 step=1 group='TerrainSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("HurstOperator",
                             m_terrainHurst,
                             "min=0 max=1.0 step=0.01 group='TerrainSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("Variance",
                             m_terrainVariance,
                             "min=0 max=2.5 step=0.01 group='TerrainSettings'"))
    {
        return false;
    }

    // Ocean Settings
    if (!m_pGUI->AddIntVar("TileFactor",
                           m_oceanTileFactor,
                           "min=1 max=10 step=1 group='OceanSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("AnimationSpeed",
                             m_oceanTimeScale,
                             "min=0 max=0.005 step=0.00001 group='OceanSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("SeaLevel",
                             m_oceanHeightOffset,
                             "min=-250 max=250 step=1 group='OceanSettings'"))
    {
        return false;
    }

    // Quad tree settings
    if (!m_pGUI->AddBoolVar("UseQuadtree", m_useQuadtree, "group='QuadTreeSettings'"))
    {
        return false;
    }
    if (!m_pGUI->AddIntVar("MaxTriangles",
                           m_maxTrianglesQtNode,
                           "min=1000 max=500000 step=10000 group='QuadTreeSettings'"))
    {
        return false;
    }

    return true;
}
