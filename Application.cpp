#include "Application.h"

#define DEBUG

Application::Application()
{
    m_fullScreen = false;
    m_vSync = false;
    m_lockSurfaceCamera = false;
    m_stopAnimation = false;
    m_leftMouseDown = false;
    m_wireframe = false;

    m_drawSkyDome = true;
    m_drawOcean = true;
    m_drawTerrain = false;

    m_oceanTimeScale = 0.0003f;
    m_oceanHeightOffset = 0.0f;

    m_screenDepth = 2500.0f;
    m_screenNear = 0.1f;
    m_spectatorHeight = 10.0f;
    m_elapsedTime = 0;

    m_pInput = 0;
    m_pDirect3D = 0;
    m_pCamera = 0;
    m_pTerrain = 0;
    m_pColorShader = 0;
    m_pTimer = 0;
    m_pPosition = 0;
    m_pUtil = 0;
    m_pTerrainShader = 0;
    m_pLight = 0;
    m_pGroundTex = 0;
    m_pSkyDomeTex = 0;
    m_pFrustum = 0;
    m_pQuadTree = 0;
    m_pFont = 0;
    m_pProfiler = 0;
    m_pSkyDome = 0;
    m_pSkyDomeShader = 0;
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
    cameraX = 50.0f;
    cameraY = 200.0f;
    cameraZ = 50.0f;
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
    result = m_pTerrain->Initialize(m_pDirect3D->GetDevice(),
                                    L"../Engine/res/terrain/heightmap01.bmp",
                                    L"../Engine/res/tex/dirt.dds",
                                    L"../Engine/res/terrain/colormap01.bmp",
                                    m_pUtil);
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
    //m_pQuadTree = new QuadTree;
    //if (!m_pQuadTree)
    //{
    //    return false;
    //}
    //result = m_pQuadTree->Initialize(m_pTerrain, m_pDirect3D->GetDevice());
    //if (!result)
    //{
    //    MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
    //    return false;
    //}

    // Create and initialize __textures__.
    /*   m_pGroundTex = new Texture;
       if (!m_pGroundTex)
       {
           return false;
       }
       result = m_pGroundTex->LoadFromDDS(m_pDirect3D->GetDevice(), L"../Engine/res/tex/dirt.dds");*/

    m_pSkyDomeTex = new Texture;
    if (!m_pSkyDomeTex)
    {
        return false;
    }
    result = m_pSkyDomeTex->LoadFromDDS(m_pDirect3D->GetDevice(), L"../Engine/res/tex/sky2.dds");


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
                                          0.4f,                 // amplitude
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

    // Create the __GUI__ for text output and GUI elements
    m_pGUI = new GUI;
    if (!m_pGUI)
    {
        return false;
    }
    if (!m_pGUI->Initialize(m_pDirect3D->GetDevice(), "Settings", screenWidth, screenHeight))
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
    if (m_pSkyDomeShader)
    {
        m_pSkyDomeShader->Shutdown();
        delete m_pSkyDomeShader;
        m_pSkyDomeShader = 0;
    }

    if (m_pSkyDome)
    {
        m_pSkyDome->Shutdown();
        delete m_pSkyDome;
        m_pSkyDome = 0;
    }

    if (m_pOceanShader)
    {
        m_pOceanShader->Shutdown();
        delete m_pOceanShader;
        m_pOceanShader = 0;
    }

    if (m_pOcean)
    {
        m_pOcean->Shutdown();
        delete m_pOcean;
        m_pOcean = 0;
    }

    if (m_pFont)
    {
        m_pFont->Shutdown();
        delete m_pFont;
        m_pFont = 0;
    }

    if (m_pProfiler)
    {
        delete m_pProfiler;
        m_pProfiler = 0;
    }

    if (m_pPosition)
    {
        delete m_pPosition;
        m_pPosition = 0;
    }

    if (m_pTimer)
    {
        delete m_pTimer;
        m_pTimer = 0;
    }

    if (m_pGroundTex)
    {
        delete m_pGroundTex;
        m_pGroundTex = 0;
    }
    if (m_pSkyDomeTex)
    {
        delete m_pSkyDomeTex;
        m_pSkyDomeTex = 0;
    }

    if (m_pQuadTree)
    {
        m_pQuadTree->Shutdown();
        delete m_pQuadTree;
        m_pQuadTree = 0;
    }

    if (m_pFrustum)
    {
        delete m_pFrustum;
        m_pFrustum = 0;
    }

    if (m_pLight)
    {
        delete m_pLight;
        m_pLight = 0;
    }

    if (m_pTerrainShader)
    {
        m_pTerrainShader->Shutdown();
        delete m_pTerrainShader;
        m_pTerrainShader = 0;
    }

    if (m_pTerrain)
    {
        m_pTerrain->Shutdown();
        delete m_pTerrain;
        m_pTerrain = 0;
    }

    if (m_pUtil)
    {
        delete m_pUtil;
        m_pUtil = 0;
    }

    if (m_pCamera)
    {
        delete m_pCamera;
        m_pCamera = 0;
    }

    if (m_pDirect3D)
    {
        m_pDirect3D->Shutdown();
        delete m_pDirect3D;
        m_pDirect3D = 0;
    }

    if (m_pInput)
    {
        m_pInput->Shutdown();
        delete m_pInput;
        m_pInput = 0;
    }

    if (m_pGUI)
    {
        m_pGUI->Shutdown();
        delete m_pGUI;
        m_pGUI = 0;
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

    // TODO
    keyDown = GetAsyncKeyState('U');
    if (keyDown)
    {
        m_pDirect3D->ToggleWireframe();
    }

    keyDown = GetAsyncKeyState('B');
    if (keyDown)
    {
        m_stopAnimation = !m_stopAnimation;
    }


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
    // Get the view point position/rotation.
    m_pPosition->GetPosition(posX, posY, posZ);
    m_pPosition->GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_pCamera->SetPosition(posX, posY, posZ);
    m_pCamera->SetRotation(rotX, rotY, rotZ);

    // Handle GUI parameters
    m_pDirect3D->SetWireframe(m_wireframe);
    m_pOcean->SetTimeScale(m_oceanTimeScale);

    return true;
}


bool Application::RenderGraphics()
{
    XMMATRIX worldMatrix;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX orthoMatrix;
    XMFLOAT3 cameraPosition;

    // Clear the scene.
    m_pDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_pCamera->Render();

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
    worldMatrix = XMMatrixTranslation(0.0f, m_oceanHeightOffset, 0.0f);


    // Render the ocean geometry
    if (m_drawOcean)
    {
        m_pOceanShader->Render(m_pDirect3D->GetDeviceContext(),
                               worldMatrix,
                               viewMatrix,
                               projectionMatrix,
                               cameraPosition,
                               m_pLight->GetDirection(),
                               m_pOcean->GetDisplacementMap(),
                               m_pOcean->GetGradientMap(),
                               m_pSkyDomeTex->GetSrv(),
                               m_wireframe);
    }

    m_pFrustum->ConstructFrustum(projectionMatrix, viewMatrix, m_screenDepth);

    //if (!m_pTerrainShader->SetShaderParameters(m_pDirect3D->GetDeviceContext(),
    //                                           worldMatrix,
    //                                           viewMatrix,
    //                                           projectionMatrix,
    //                                           m_pGroundTex->GetSrv(),
    //                                           m_pLight->GetDirection(),
    //                                           m_pLight->GetAmbientColor(),
    //                                           m_pLight->GetDiffuseColor() ))
    //{
    //    return false;
    //}

    // Render the terrain using the quad tree and terrain shader.
    //m_pQuadTree->Render(m_pFrustum, m_pDirect3D->GetDeviceContext(), m_pTerrainShader);

// profiling/debug output
#ifdef DEBUG
    int fps = m_pProfiler->GetFps();

    std::wostringstream fpswchar;
    fpswchar << fps << " FPS";

    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      (WCHAR *)fpswchar.str().c_str(),
                      13.0f,
                      20.0f,
                      20.0f,
                      0xff8cc63e,
                      0);

    //std::wostringstream triangleswchar;
    //triangleswchar << m_pQuadTree->GetDrawCount() << " Tris";
    //m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
    //                  (WCHAR *)triangleswchar.str().c_str(),
    //                  13.0f,
    //                  20.0f,
    //                  38.0f,
    //                  0xff8cc63e,
    //                  0);
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


bool Application::SetGuiParams()
{
    // not working yet, re-initialization of D3D required??
    //m_pGUI->AddBoolVar("vSync", m_vSync);
    //m_pGUI->AddBoolVar("FullScreen", m_fullScreen);

    if (!m_pGUI->AddBoolVar("Pause", m_stopAnimation))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("Wireframe", m_wireframe))
    {
        return false;
    }

    if (!m_pGUI->AddBoolVar("RenderSky", m_drawSkyDome))
    {
        return false;
    }
    if (!m_pGUI->AddBoolVar("RenderOcean", m_drawOcean))
    {
        return false;
    }

    if (!m_pGUI->AddFloatVar("AnimationSpeed", m_oceanTimeScale, " min=0 max=0.005 step=0.00001 "))
    {
        return false;
    }
    if (!m_pGUI->AddFloatVar("SeaLevel", m_oceanHeightOffset, " min=-150 max=150 step=1 "))
    {
        return false;
    }

    return true;
}
