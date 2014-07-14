#include "Application.h"

#define DEBUG

Application::Application()
{
    m_fullScreen = false;
    m_vSync = true;
    m_lockSurfaceCamera = false;

    m_screenDepth = 1000.0f;
    m_screenNear = 0.1f;
    m_spectatorHeight = 10.0f;

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

    // setup for 16 bit alignment used by XMMATH for SSE instruction support
    // TODO: overload <new> and <delete> operators
    void *ptr = 0;
    size_t alignment = 16;
    size_t objSize = sizeof(D3D);

    // Create and initialize _D3D_ window object.
    ptr = _aligned_malloc(objSize, alignment);
    m_pDirect3D = new(ptr)D3D();
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
    ptr = _aligned_malloc(objSize, alignment);
    m_pCamera = new(ptr)Camera();
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
    m_pLight->SetDirection(-0.5f, -1.0f, 0.0f);

    // Create the __frustum__ for culling
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
                      L"Loading Terrain Data...\n",
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
    result = m_pQuadTree->Initialize(m_pTerrain, m_pDirect3D->GetDevice());
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize __texture__.
    m_pGroundTex = new Texture;
    if (!m_pGroundTex)
    {
        return false;
    }
    result = m_pGroundTex->LoadFromDDS(m_pDirect3D->GetDevice(), L"../Engine/res/tex/dirt.dds");

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
    Ocean::OceanParameter oceanParams = { 512,
                                          0.8f,
                                          0.35f,
                                          Vec2f(0.8f, 0.6f),
                                          600.0f,
                                          0.07f,
                                          1.3f
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
    // TODO: Ocean shading

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
        //delete m_pCamera;
        m_pCamera = 0;
    }
    // TODO
    if (m_pDirect3D)
    {
        //m_pDirect3D->Shutdown();
        //delete m_pDirect3D;
        m_pDirect3D = 0;
    }

    if (m_pInput)
    {
        m_pInput->Shutdown();
        delete m_pInput;
        m_pInput = 0;
    }

    return;
}

bool Application::ProcessFrame()
{
    bool result = false;
    float height;

    // Update the system stats.
    m_pTimer->Frame();
    m_pProfiler->Frame();

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
        result = m_pQuadTree->GetHeightAtPosition(position.x, position.z, height);
        if (result)
        {
            m_pCamera->SetPosition(position.x, height + m_spectatorHeight, position.z);
        }
    }

    // Ocean displacement
    m_pOcean->UpdateDisplacement(m_pTimer->GetTime(), m_pDirect3D->GetDeviceContext());

    // Render the graphics.
    if (!RenderGraphics())
    {
        return false;
    }

    return result;
}


bool Application::HandleInput(float frameTime)
{
    short keyDown = false;
    bool moveCamOnDrag = false;

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
    keyDown = GetAsyncKeyState(VK_F1);
    if (keyDown)
    {
        m_pDirect3D->ToggleWireframe();
    }

    // Yaw and pitch with __mouse__ movement.
    if (moveCamOnDrag)
    {
        // not working properly yet
        if (DragDetect(m_hwnd, m_pInput->GetMousePoint()))
        {
            m_pInput->GetMouseLocationChage(mouseX, mouseY);
            m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.2f);
        }
    }
    else
    {
        m_pInput->GetMouseLocationChage(mouseX, mouseY);
        m_pPosition->TurnOnMouseMovement(mouseX, mouseY, 0.2f);
    }
    // Get the view point position/rotation.
    m_pPosition->GetPosition(posX, posY, posZ);
    m_pPosition->GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_pCamera->SetPosition(posX, posY, posZ);
    m_pCamera->SetRotation(rotX, rotY, rotZ);

    return true;
}


bool Application::RenderGraphics()
{
    XMMATRIX worldMatrix;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX orthoMatrix;
    XMFLOAT3 cameraPosition;

    bool result;

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
    worldMatrix = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

    m_pDirect3D->TurnOffCulling();
    m_pDirect3D->TurnZBufferOff();

    // Render the sky dome.
    m_pSkyDome->Render(m_pDirect3D->GetDeviceContext());
    m_pSkyDomeShader->Render(m_pDirect3D->GetDeviceContext(),
                             m_pSkyDome->GetIndexCount(),
                             worldMatrix,
                             viewMatrix,
                             projectionMatrix,
                             m_pSkyDome->GetApexColor(),
                             m_pSkyDome->GetCenterColor());

    m_pDirect3D->TurnOnCulling();
    m_pDirect3D->TurnZBufferOn();

    // Reset the world matrix.
    m_pDirect3D->GetWorldMatrix(worldMatrix);

    m_pFrustum->ConstructFrustum(projectionMatrix, viewMatrix, m_screenDepth);

    result = m_pTerrainShader->SetShaderParameters(m_pDirect3D->GetDeviceContext(),
                                                   worldMatrix,
                                                   viewMatrix,
                                                   projectionMatrix,
                                                   m_pGroundTex->GetSrv(),
                                                   m_pLight->GetDirection(),
                                                   m_pLight->GetAmbientColor(),
                                                   m_pLight->GetDiffuseColor() );

    if (!result)
    {
        return false;
    }

    // Render the terrain using the quad tree and terrain shader.
    m_pQuadTree->Render(m_pFrustum, m_pDirect3D->GetDeviceContext(), m_pTerrainShader);


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

    std::wostringstream triangleswchar;
    triangleswchar << m_pQuadTree->GetDrawCount() << " Tris";
    m_pFont->drawText(m_pDirect3D->GetDeviceContext(),
                      (WCHAR *)triangleswchar.str().c_str(),
                      13.0f,
                      20.0f,
                      38.0f,
                      0xff8cc63e,
                      0);
#endif


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