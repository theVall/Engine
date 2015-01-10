#include "Graphics.h"


Graphics::Graphics(void)
{
    m_fullScreen = false;
    m_vSync = true;
    m_screenDepth = 1000.0f;
    m_screenNear = 0.1f;

    m_D3D = 0;
    m_Camera = 0;
    m_Model = 0;
    m_TextureShader = 0;
    m_LightShader = 0;
    m_Light = 0;
    m_Element2d = 0;
}


Graphics::Graphics(const Graphics &)
{
}


Graphics::~Graphics()
{
}


bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool success = false;


    // Create D3D window object.
    m_D3D = new D3D();
    if(!m_D3D)
    {
        return false;
    }
    success = m_D3D->Initialize(screenWidth,
                                screenHeight,
                                m_vSync,
                                hwnd,
                                m_fullScreen,
                                m_screenDepth,
                                m_screenNear);
    if(!success)
    {
        MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    // Create camera and set initial position.
    m_Camera = new Camera();
    if (!m_Camera)
    {
        return false;
    }
    m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

    // Load models.
    m_Model = new Model;
    if (!m_Model)
    {
        return false;
    }
    success = m_Model->Initialize(m_D3D->GetDevice(),
                                  L"../Engine/res/model/cube.txt",
                                  L"../Engine/res/tex/seafloor.dds");
    if (!success)
    {
        MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
        return false;
    }

    // Load shaders.
    m_LightShader = new LightShader;
    if (!m_LightShader)
    {
        return false;
    }

    success = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
    if (!success)
    {
        MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
        return false;
    }

    // Load light.
    m_Light = new Light;
    if (!m_Light)
    {
        return false;
    }
    m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetSpecularPower(32.0f);
    m_Light->SetDirection(1.0f, 0.0f, 0.0f);

    return true;
}


void Graphics::Shutdown()
{
    if (m_Light)
    {
        delete m_Light;
        m_Light = 0;
    }

    if (m_LightShader)
    {
        m_LightShader->Shutdown();
        delete m_LightShader;
        m_LightShader = 0;
    }

    if (m_TextureShader)
    {
        m_TextureShader->Shutdown();
        delete m_TextureShader;
        m_TextureShader = 0;
    }

    if (m_Model)
    {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = 0;
    }

    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = 0;
    }

    if(m_D3D)
    {
        m_D3D->Shutdown();
        delete m_D3D;
        m_D3D = 0;
    }

    return;
}


bool Graphics::ProcessFrame()
{
    bool success;
    static float rotation = 0.0f;

    rotation += (float)XM_PI * 0.005f;
    if (rotation > 360.0f)
    {
        rotation -= 360.0f;
    }

    success = Render(rotation);
    if(!success)
    {
        return false;
    }

    return true;
}


bool Graphics::Render(float rotation)
{
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX worldMatrix;

    bool result;

    // Clear buffers.
    m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on camera position.
    m_Camera->Render();

    // Get the world, view, and projection matrices.
    m_D3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_D3D->GetProjectionMatrix(projectionMatrix);

    // Rotation
    worldMatrix = XMMatrixMultiply(XMMatrixRotationY(rotation), worldMatrix);
    worldMatrix = XMMatrixMultiply(XMMatrixRotationX(rotation), worldMatrix);

    // Put the model vertex and index buffers on the graphics pipeline.
    m_Model->Render(m_D3D->GetDeviceContext());

    result = m_LightShader->Render(m_D3D->GetDeviceContext(),
                                   m_Model->GetIndexCount(),
                                   worldMatrix,
                                   viewMatrix,
                                   projectionMatrix,
                                   m_Model->GetTexture(),
                                   m_Light->GetDirection(),
                                   m_Light->GetAmbientColor(),
                                   m_Light->GetDiffuseColor(),
                                   m_Light->GetSpecularColor(),
                                   m_Light->GetSpecularPower(),
                                   m_Camera->GetPosition().GetAsXMFloat3());
    if (!result)
    {
        return false;
    }

    m_D3D->EndScene();

    return true;
}


bool Graphics::IsFullScreen()
{
    return m_fullScreen;
}


bool Graphics::IsVsync()
{
    return m_vSync;
}


float Graphics::GetScreenDepth()
{
    return m_screenDepth;
}


float Graphics::GetScreenNear()
{
    return m_screenNear;
}
