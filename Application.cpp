#include "Application.h"


Application::Application()
{
    m_fullScreen = false;
    m_vSync = true;
    m_screenDepth = 1000.0f;
    m_screenNear = 0.1f;

    m_Input = 0;
    m_Direct3D = 0;
    m_Camera = 0;
    m_Terrain = 0;
    m_ColorShader = 0;
    m_Timer = 0;
    m_Position = 0;
    m_ImageUtil = 0;
    m_TerrainShader = 0;
    m_Light = 0;
    m_Texture = 0;
    m_Frustum = 0;
    m_QuadTree = 0;
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

    // Create and initialize the __Direct3D__ object.
    m_Direct3D = new D3D;
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
    m_Camera = new Camera;
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
    m_ImageUtil = new ImageUtil;
    if (!m_ImageUtil)
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
                                   m_ImageUtil);
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
    result = m_TerrainShader->Initialize(m_Direct3D->GetDevice(), m_hwnd);
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
    m_Texture = new Texture;
    if (!m_Texture)
    {
        return false;
    }
    result = m_Texture->Initialize(m_Direct3D->GetDevice(), L"../Engine/res/tex/dirt.dds");

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
        return false;
    }
    // Set the initial position of the viewer to the same as the initial camera position.
    m_Position->SetPosition(cameraX, cameraY, cameraZ);

    // FPS
    // CPU
    // FONT SHADER
    // TEXT

    return true;
}


void Application::Shutdown()
{
    //// Release the text object.
    //if (m_Text)
    //{
    //    m_Text->Shutdown();
    //    delete m_Text;
    //    m_Text = 0;
    //}

    //// Release the font shader object.
    //if (m_FontShader)
    //{
    //    m_FontShader->Shutdown();
    //    delete m_FontShader;
    //    m_FontShader = 0;
    //}

    //// Release the CPU object.
    //if (m_Cpu)
    //{
    //    m_Cpu->Shutdown();
    //    delete m_Cpu;
    //    m_Cpu = 0;
    //}

    //// Release the fps object.
    //if (m_Fps)
    //{
    //    delete m_Fps;
    //    m_Fps = 0;
    //}

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

    if (m_Texture)
    {
        delete m_Texture;
        m_Texture = 0;
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

    if (m_ImageUtil)
    {
        delete m_ImageUtil;
        m_ImageUtil = 0;
    }

    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = 0;
    }

    if (m_Direct3D)
    {
        m_Direct3D->Shutdown();
        delete m_Direct3D;
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
    bool result;

    // Read the user input.
    //result = m_Input->Frame();
    //if (!result)
    //{
    //    return false;
    //}

    // Check if the user pressed escape and wants to exit the application.
    //if (m_Input->IsEscapePressed() == true)
    //{
    //    return false;
    //}

    // Update the system stats.
    m_Timer->Frame();
    //m_Fps->Frame();
    //m_Cpu->Frame();

    //// Update the FPS value in the text object.
    //result = m_Text->SetFps(m_Fps->GetFps(), m_Direct3D->GetDeviceContext());
    //if (!result)
    //{
    //    return false;
    //}

    //// Update the CPU usage value in the text object.
    //result = m_Text->SetCpu(m_Cpu->GetCpuPercentage(), m_Direct3D->GetDeviceContext());
    //if (!result)
    //{
    //    return false;
    //}

    // Do the frame input processing.
    result = HandleInput(m_Timer->GetTime());
    if (!result)
    {
        return false;
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
    int keyDown = false;
    bool result = false;
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
    m_Position->MoveUpward(keyDown, sensitivity);

    keyDown = GetAsyncKeyState('C');
    m_Position->MoveDownward(keyDown, sensitivity);

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

    //// Update the position values in the text object.
    //result = m_Text->SetCameraPosition(posX, posY, posZ, m_Direct3D->GetDeviceContext());
    //if (!result)
    //{
    //    return false;
    //}

    //// Update the rotation values in the text object.
    //result = m_Text->SetCameraRotation(rotX, rotY, rotZ, m_Direct3D->GetDeviceContext());
    //if (!result)
    //{
    //    return false;
    //}

    return true;
}


bool Application::RenderGraphics()
{
    XMMATRIX worldMatrix;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX orthoMatrix;

    bool result;

    // Clear the scene.
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_Camera->Render();

    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);
    m_Direct3D->GetOrthoMatrix(orthoMatrix);

    m_Frustum->ConstructFrustum(projectionMatrix, viewMatrix, m_screenDepth);

    result = m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(),
                                                  worldMatrix,
                                                  viewMatrix,
                                                  projectionMatrix,
                                                  m_Texture->GetTexture(),
                                                  m_Light->GetDirection(),
                                                  m_Light->GetAmbientColor(),
                                                  m_Light->GetDiffuseColor() );
    
    if (!result)
    {
        return false;
    }

    // Render the terrain using the quad tree and terrain shader.
    m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);


    char char1[24];
    char char2[24];
    char char3[24];
    char char4[24];
    _itoa_s(m_QuadTree->GetDrawCount(), char1, (size_t) 24, 10);
    LPCSTR s = char1;
    OutputDebugStringA(s);
    OutputDebugStringW(TEXT("\n"));

    //XMFLOAT3 vec = m_Camera->GetPosition();

    //_itoa_s(vec.x, char2, (size_t)24, 10);
    //_itoa_s(vec.y, char3, (size_t)24, 10);
    //_itoa_s(vec.z, char4, (size_t)24, 10);

    //OutputDebugStringW(TEXT("x: "));
    //s = char2;
    //OutputDebugStringA(s);
    //OutputDebugStringW(TEXT("  y: "));
    //s = char3;
    //OutputDebugStringA(s);
    //OutputDebugStringW(TEXT("  z: "));
    //s = char4;
    //OutputDebugStringA(s);
    //OutputDebugStringW(TEXT("\n"));


    // Turn off the Z buffer to begin all 2D rendering.
    //m_Direct3D->TurnZBufferOff();

    // Turn on the alpha blending before rendering the text.
//    m_Direct3D->TurnOnAlphaBlending();

    //// Render the text user interface elements.
    //result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix);
    //if (!result)
    //{
    //    return false;
    //}

    // Turn off alpha blending after rendering the text.
//    m_Direct3D->TurnOffAlphaBlending();

    // Turn the Z buffer back on now that all 2D rendering has completed.
    //m_Direct3D->TurnZBufferOn();

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