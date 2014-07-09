#include "Application.h"


Application::Application()
{
    m_fullScreen = false;
    m_vSync = true;
    m_lockSurfaceCamera = false;

    m_screenDepth = 1000.0f;
    m_screenNear = 0.1f;
    m_SpectatorHeight = 10.0f;

    m_Input = 0;
    m_Direct3D = 0;
    m_Camera = 0;
    m_Terrain = 0;
    m_ColorShader = 0;
    m_Timer = 0;
    m_Position = 0;
    m_Util = 0;
    m_TerrainShader = 0;
    m_Light = 0;
    m_pGroundTex = 0;
    m_Frustum = 0;
    m_QuadTree = 0;
    m_Font = 0;
    m_Profiler = 0;
    m_SkyDome = 0;
    m_SkyDomeShader = 0;
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
    m_Input = new Input;
    if (!m_Input)
    {
        return false;
    }
    result = m_Input->Initialize(m_hwnd, screenWidth, screenHeight);
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
    m_Direct3D = new(ptr)D3D();
    if (!m_Direct3D)
    {
        return false;
    }
    result = m_Direct3D->Initialize(screenWidth,
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
    m_Camera = new(ptr)Camera();
    if (!m_Camera)
    {
        return false;
    }
    // Initialize a base view matrix with the camera for 2D user interface rendering.
    m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
    m_Camera->Render();
    m_Camera->GetViewMatrix(baseViewMatrix);
    // Set the initial position of the camera.
    cameraX = 50.0f;
    cameraY = 200.0f;
    cameraZ = 50.0f;
    m_Camera->SetPosition(cameraX, cameraY, cameraZ);

    // image loading utility object
    m_Util = new Util;
    if (!m_Util)
    {
        return false;
    }

    // Create and initialize the __terrain__ object.
    m_Terrain = new Terrain;
    if (!m_Terrain)
    {
        return false;
    }
    result = m_Terrain->Initialize(m_Direct3D->GetDevice(),
                                   L"../Engine/res/terrain/heightmap01.bmp",
                                   L"../Engine/res/tex/dirt.dds",
                                   L"../Engine/res/terrain/colormap01.bmp",
                                   m_Util);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize __shader__.
    m_TerrainShader = new TerrainShader;
    if (!m_TerrainShader)
    {
        return false;
    }
    result = m_TerrainShader->Initialize(m_Direct3D->GetDevice(),
                                         m_hwnd,
                                         L"../Engine/shader/TerrainVS.hlsl",
                                         L"../Engine/shader/TerrainPS.hlsl");
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize __light__.
    m_Light = new Light;
    if (!m_Light)
    {
        return false;
    }
    m_Light->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(-0.5f, -1.0f, 0.0f);

    // Create the __frustum__ for culling
    m_Frustum = new Frustum;
    if (!m_Frustum)
    {
        return false;
    }

    // Create and initialize the __font wrapper__ object.
    m_Font = new Font;
    if (!m_Font)
    {
        MessageBox(m_hwnd, L"Could not font wrapper object.", L"Error", MB_OK);
        return false;
    }
    m_Font->Initialize(L"Arial", m_Direct3D->GetDevice());

    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    m_Font->drawText(m_Direct3D->GetDeviceContext(),
                     L"Loading Terrain Data...\n",
                     20.0f,
                     50.0f,
                     50.0f,
                     0xff8cc63e,
                     0);
    m_Direct3D->EndScene();

    // Create and initialize the __quad tree__.
    m_QuadTree = new QuadTree;
    if (!m_QuadTree)
    {
        return false;
    }
    result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice());
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
    result = m_pGroundTex->LoadFromDDS(m_Direct3D->GetDevice(), L"../Engine/res/tex/dirt.dds");

    // Create and initialize the __timer__ object.
    m_Timer = new Timer;
    if (!m_Timer)
    {
        return false;
    }
    result = m_Timer->Initialize();
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __position__ object.
    m_Position = new Position;
    if (!m_Position)
    {
        MessageBox(m_hwnd, L"Could not initialize the position object.", L"Error", MB_OK);
        return false;
    }
    // Set the initial position of the viewer to the same as the initial camera position.
    m_Position->SetPosition(cameraX, cameraY, cameraZ);

    // Create and initialize the __profiler__ object.
    m_Profiler = new Profiler;
    if (!m_Profiler)
    {
        MessageBox(m_hwnd, L"Could not initialize the profiling object.", L"Error", MB_OK);
        return false;
    }
    m_Profiler->Initialize();

    // Create and initialize the __sky dome__ object.
    m_SkyDome = new SkyDome;
    if (!m_SkyDome)
    {
        return false;
    }
    if (!m_SkyDome->Initialize(m_Direct3D->GetDevice(), L"../Engine/res/model/dome.txt", m_Util))
    {
        MessageBox(hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
        return false;
    }

    // Create and initialize the __sky dome shader__ object.
    m_SkyDomeShader = new SkyDomeShader;
    if (!m_SkyDomeShader)
    {
        return false;
    }
    if (!m_SkyDomeShader->Initialize(m_Direct3D->GetDevice(),
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
                              m_Direct3D->GetDevice(),
                              m_Direct3D->GetDeviceContext(),
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
    if (m_SkyDomeShader)
    {
        m_SkyDomeShader->Shutdown();
        delete m_SkyDomeShader;
        m_SkyDomeShader = 0;
    }

    if (m_SkyDome)
    {
        m_SkyDome->Shutdown();
        delete m_SkyDome;
        m_SkyDome = 0;
    }

    if (m_Font)
    {
        m_Font->Shutdown();
        delete m_Font;
        m_Font = 0;
    }

    if (m_Profiler)
    {
        delete m_Profiler;
        m_Profiler = 0;
    }

    if (m_Position)
    {
        delete m_Position;
        m_Position = 0;
    }

    if (m_Timer)
    {
        delete m_Timer;
        m_Timer = 0;
    }

    if (m_pGroundTex)
    {
        delete m_pGroundTex;
        m_pGroundTex = 0;
    }

    if (m_QuadTree)
    {
        m_QuadTree->Shutdown();
        delete m_QuadTree;
        m_QuadTree = 0;
    }

    if (m_Frustum)
    {
        delete m_Frustum;
        m_Frustum = 0;
    }

    if (m_Light)
    {
        delete m_Light;
        m_Light = 0;
    }

    if (m_TerrainShader)
    {
        m_TerrainShader->Shutdown();
        delete m_TerrainShader;
        m_TerrainShader = 0;
    }

    if (m_Terrain)
    {
        m_Terrain->Shutdown();
        delete m_Terrain;
        m_Terrain = 0;
    }

    if (m_Util)
    {
        delete m_Util;
        m_Util = 0;
    }

    if (m_Camera)
    {
        //delete m_Camera;
        m_Camera = 0;
    }
    // TODO
    if (m_Direct3D)
    {
        //m_Direct3D->Shutdown();
        //delete m_Direct3D;
        m_Direct3D = 0;
    }

    if (m_Input)
    {
        m_Input->Shutdown();
        delete m_Input;
        m_Input = 0;
    }

    return;
}

bool Application::ProcessFrame()
{
    bool result = false;
    float height;

    // Update the system stats.
    m_Timer->Frame();
    m_Profiler->Frame();

    // Do the frame input processing.
    result = HandleInput(m_Timer->GetTime());
    if (!result)
    {
        return false;
    }

    if (m_lockSurfaceCamera)
    {
        Vec3f position = m_Camera->GetPosition();
        // Get the height of the triangle that is directly underneath the camera position.
        // If there was a triangle under the camera position,
        // set the camera two units above it.
        result = m_QuadTree->GetHeightAtPosition(position.x, position.z, height);
        if (result)
        {
            m_Camera->SetPosition(position.x, height + m_SpectatorHeight, position.z);
        }
    }

    // Render the graphics.
    result = RenderGraphics();
    if (!result)
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
    m_Position->SetFrameTime(frameTime);

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
    m_Position->MoveForward((keyDown == 1) || (keyDown == 0x8000), sensitivity);

    keyDown = GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A');
    m_Position->MoveLeft((keyDown == 1) || (keyDown == 0x8000), sensitivity);

    keyDown = GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S');
    m_Position->MoveBackward((keyDown == 1) || (keyDown == 0x8000), sensitivity);

    keyDown = GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D');
    m_Position->MoveRight((keyDown == 1) || (keyDown == 0x8000), sensitivity);

    keyDown = GetAsyncKeyState(VK_SPACE);
    m_Position->MoveUpward(keyDown != 0, sensitivity);

    keyDown = GetAsyncKeyState('C');
    m_Position->MoveDownward(keyDown != 0, sensitivity);

    // TODO
    keyDown = GetAsyncKeyState(VK_F1);
    if (keyDown)
    {
        m_Direct3D->ToggleWireframe();
    }

    // Yaw and pitch with __mouse__ movement.
    if (moveCamOnDrag)
    {
        // not working properly yet
        if (DragDetect(m_hwnd, m_Input->GetMousePoint()))
        {
            m_Input->GetMouseLocationChage(mouseX, mouseY);
            m_Position->TurnOnMouseMovement(mouseX, mouseY, 0.2f);
        }
    }
    else
    {
        m_Input->GetMouseLocationChage(mouseX, mouseY);
        m_Position->TurnOnMouseMovement(mouseX, mouseY, 0.2f);
    }
    // Get the view point position/rotation.
    m_Position->GetPosition(posX, posY, posZ);
    m_Position->GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_Camera->SetPosition(posX, posY, posZ);
    m_Camera->SetRotation(rotX, rotY, rotZ);

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
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_Camera->Render();

    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);
    m_Direct3D->GetOrthoMatrix(orthoMatrix);

    // Sky dome calculations
    cameraPosition = m_Camera->GetPosition();
    worldMatrix = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

    m_Direct3D->TurnOffCulling();
    m_Direct3D->TurnZBufferOff();

    // Render the sky dome.
    m_SkyDome->Render(m_Direct3D->GetDeviceContext());
    m_SkyDomeShader->Render(m_Direct3D->GetDeviceContext(),
                            m_SkyDome->GetIndexCount(),
                            worldMatrix,
                            viewMatrix,
                            projectionMatrix,
                            m_SkyDome->GetApexColor(),
                            m_SkyDome->GetCenterColor());

    m_Direct3D->TurnOnCulling();
    m_Direct3D->TurnZBufferOn();

    // Reset the world matrix.
    m_Direct3D->GetWorldMatrix(worldMatrix);

    m_Frustum->ConstructFrustum(projectionMatrix, viewMatrix, m_screenDepth);

    result = m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(),
                                                  worldMatrix,
                                                  viewMatrix,
                                                  projectionMatrix,
                                                  m_pGroundTex->GetSrv(),
                                                  m_Light->GetDirection(),
                                                  m_Light->GetAmbientColor(),
                                                  m_Light->GetDiffuseColor() );

    if (!result)
    {
        return false;
    }

    // Render the terrain using the quad tree and terrain shader.
    m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);


// profiling/debug output
#ifdef DEBUG
    int fps = m_Profiler->GetFps();

    std::wostringstream fpswchar;
    fpswchar << fps << " FPS";

    m_Font->drawText(m_Direct3D->GetDeviceContext(),
                     (WCHAR *)fpswchar.str().c_str(),
                     13.0f,
                     20.0f,
                     20.0f,
                     0xff8cc63e,
                     0);

    std::wostringstream triangleswchar;
    triangleswchar << m_QuadTree->GetDrawCount() << " Tris";
    m_Font->drawText(m_Direct3D->GetDeviceContext(),
                     (WCHAR *)triangleswchar.str().c_str(),
                     13.0f,
                     20.0f,
                     38.0f,
                     0xff8cc63e,
                     0);
#endif


    // Present the rendered scene to the screen.
    m_Direct3D->EndScene();

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