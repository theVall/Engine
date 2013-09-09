#include "Graphics.h"


Graphics::Graphics(void)
{
    m_fullScreen = false;
    m_vSync = true;
    m_screenDepth = 1000.0f;
    m_screenNear = 0.1f;

    m_D3D = 0;
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

    m_D3D = new D3D;
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

    return true;
}


void Graphics::Shutdown()
{
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

    success = Render();
    if(!success)
    {
        return false;
    }

    return true;
}


bool Graphics::Render()
{
    //  Clear the scene.
    m_D3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

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
