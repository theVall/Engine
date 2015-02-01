#include "D3D.h"


D3D::D3D(void)
{
    m_swapChain            = 0;
    m_device               = 0;
    m_deviceContext        = 0;
    m_renderTargetView     = 0;
    m_depthStencilBuffer   = 0;
    m_depthStencilState    = 0;
    m_depthStencilView     = 0;

    m_rasterStateSolid     = 0;
    m_rasterStateWireFrame = 0;
    m_rasterStateNoCulling = 0;

    m_depthDisabledStencilState = 0;
}


D3D::D3D(const D3D &)
{
}


D3D::~D3D(void)
{
}


bool D3D::Initialize(int screenWidth,
                     int screenHeight,
                     bool vSync,
                     HWND hwnd,
                     bool fullscreen,
                     float screenDepth,
                     float screenNear)
{
    HRESULT result;
    IDXGIFactory1 *factory;
    IDXGIAdapter *adapter;
    IDXGIOutput *adapterOutput;
    DXGI_MODE_DESC *displayModeList;
    DXGI_ADAPTER_DESC adapterDesc;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Texture2D *backBufferPtr;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;

    unsigned int numModes     = 0;
    unsigned int i            = 0;
    unsigned int numerator    = 0;
    unsigned int denominator  = 0;
    unsigned int stringLength = 0;
    int error                 = 0;
    float fovAngleY           = 0.0f;
    float screenAspect        = 0.0f;

    m_vSyncEnabled = vSync;

    //  Create a DirectX graphics interface factory.
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory);
    if (FAILED(result))
    {
        return false;
    }

    //  Use the factory to create an adapter for the primary graphics interface.
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result))
    {
        return false;
    }

    // Enumerate the primary adapter output (monitor).
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result))
    {
        return false;
    }

    //  Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format
    //  for the adapter output (monitor).
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                               DXGI_ENUM_MODES_INTERLACED,
                                               &numModes,
                                               NULL);
    if (FAILED(result))
    {
        return false;
    }

    //  Create a list to hold all the possible display modes for this monitor/GPU
    //  combination.
    displayModeList = new DXGI_MODE_DESC[numModes];
    if(!displayModeList)
    {
        return false;
    }

    //  Fill the display mode list structures.
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                               DXGI_ENUM_MODES_INTERLACED,
                                               &numModes,
                                               displayModeList);
    if (FAILED(result))
    {
        return false;
    }

    // Find the display mode that matches the screen width and height. When a match is
    // found store the numerator and denominator of the refresh rate for that monitor.
    for (i = 0; i < numModes; ++i)
    {
        if (displayModeList[i].Width == (unsigned int)screenWidth)
        {
            if (displayModeList[i].Height == (unsigned int)screenHeight)
            {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result))
    {
        return false;
    }

    //  Store the dedicated video card memory in megabytes.
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    //  Convert the name of the video card to a character array and store it.
    error = wcstombs_s(&stringLength,
                       m_videoCardDescription,
                       128,
                       adapterDesc.Description,
                       128);
    if(error != 0)
    {
        return false;
    }

    //  Release structures and interfaces used to gather the system information.
    delete [] displayModeList;
    displayModeList = 0;

    adapterOutput->Release();
    adapterOutput = 0;

    adapter->Release();
    adapter = 0;

    factory->Release();
    factory = 0;

    //  Initialize the swap chain and set the back buffer.
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    swapChainDesc.BufferCount = 1;

    swapChainDesc.BufferDesc.Width  = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    //  Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    //  Set the refresh rate of the back buffer.
    if (m_vSyncEnabled)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    swapChainDesc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;

    //  Turn multisampling off.
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    //  Set to full screen or windowed mode.
    if (fullscreen)
    {
        swapChainDesc.Windowed = false;
    }
    else
    {
        swapChainDesc.Windowed = true;
    }

    //  Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //  Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    //  Don't set the advanced flags.
    swapChainDesc.Flags = 0;

    featureLevel = D3D_FEATURE_LEVEL_11_0;

    //  Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(NULL,
                                           D3D_DRIVER_TYPE_HARDWARE,
                                           NULL,
                                           D3D11_CREATE_DEVICE_DEBUG,
                                           &featureLevel,
                                           1,
                                           D3D11_SDK_VERSION,
                                           &swapChainDesc,
                                           &m_swapChain,
                                           &m_device,
                                           NULL,
                                           &m_deviceContext);

    if (FAILED(result))
    {
        return false;
    }

    //  Get the pointer to the back buffer.
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) &backBufferPtr);
    if (FAILED(result))
    {
        return false;
    }

    result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    if (FAILED(result))
    {
        return false;
    }

    backBufferPtr->Release();
    backBufferPtr = 0;

    //  DEPTH BUFFER
    //  Initialize and set up the description of the depth buffer.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    depthBufferDesc.Width               = screenWidth;
    depthBufferDesc.Height              = screenHeight;
    depthBufferDesc.MipLevels           = 1;
    depthBufferDesc.ArraySize           = 1;
    depthBufferDesc.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count    = 1;
    depthBufferDesc.SampleDesc.Quality  = 0;
    depthBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags      = 0;
    depthBufferDesc.MiscFlags           = 0;

    //  Create the texture for the depth buffer using the description.
    result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result))
    {
        return false;
    }

    //  DEPTH STENCIL
    //  Initialize and set up the description of the stencil state.
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    depthStencilDesc.DepthEnable        = true;
    depthStencilDesc.DepthWriteMask     = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc          = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable      = true;
    depthStencilDesc.StencilReadMask    = 0xFF;
    depthStencilDesc.StencilWriteMask   = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    //  Create the depth stencil state.
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result))
    {
        return false;
    }

    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    //  Initialize and set up the depth stencil view.
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = m_device->CreateDepthStencilView(m_depthStencilBuffer,
                                              &depthStencilViewDesc,
                                              &m_depthStencilView);
    if (FAILED(result))
    {
        return false;
    }

    // Bind the render target view and depth stencil buffer to the output render pipeline.
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // RASTERIZER
    // Setup the raster description for polygon draw manipulation.
    // Default state with back-face culling and solid fill mode.
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode              = D3D11_CULL_BACK;
    rasterDesc.DepthBias             = 0;
    rasterDesc.DepthBiasClamp        = 0.0f;
    rasterDesc.DepthClipEnable       = true;
    rasterDesc.FillMode              = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable     = false;
    rasterDesc.ScissorEnable         = false;
    rasterDesc.SlopeScaledDepthBias  = 0.0f;

    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterStateSolid);
    if (FAILED(result))
    {
        return false;
    }

    // No back face culling state
    rasterDesc.CullMode = D3D11_CULL_NONE;
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterStateNoCulling);
    if (FAILED(result))
    {
        return false;
    }

    // Wire frame state
    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterStateWireFrame);
    if (FAILED(result))
    {
        return false;
    }

    m_deviceContext->RSSetState(m_rasterStateSolid);

    //  VIEWPORT as entire window size.
    viewport.Width    = (float) screenWidth;
    viewport.Height   = (float) screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    m_deviceContext->RSSetViewports(1, &viewport);

    //  MATRICES
    fovAngleY    = (float) XM_PI / 4.0f;
    screenAspect = (float) screenWidth / (float) screenHeight;

    //  Build a left-handed perspective projection matrix.
    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovAngleY,
                                                           screenAspect,
                                                           screenNear,
                                                           screenDepth);
    //  Initialize wold matrix as identity.
    m_worldMatrix = XMMatrixIdentity();

    //  Create an orthographic projection matrix for 2D rendering.
    m_orthoMatrix = XMMatrixOrthographicLH((float) screenWidth,
                                           (float) screenHeight,
                                           screenNear,
                                           screenDepth);

    // Clear the second depth stencil state before setting the parameters.
    ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

    // Create a second depth stencil state which turns off the Z buffer for 2D rendering.
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.StencilEnable = true;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the state using the device.
    result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc,
                                               &m_depthDisabledStencilState);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void D3D::Shutdown()
{
    // Before shutting down set to windowed mode.
    if (m_swapChain)
    {
        m_swapChain->SetFullscreenState(false, NULL);
    }

    SafeRelease(m_depthDisabledStencilState);
    SafeRelease(m_rasterStateSolid);
    SafeRelease(m_rasterStateWireFrame);
    SafeRelease(m_rasterStateNoCulling);
    SafeRelease(m_depthStencilView);
    SafeRelease(m_depthStencilState);
    SafeRelease(m_depthStencilBuffer);
    SafeRelease(m_renderTargetView);
    SafeRelease(m_deviceContext);
    SafeRelease(m_device);
    SafeRelease(m_swapChain);

    return;
}


void D3D::BeginScene(float red, float green, float blue, float alpha)
{
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    //  Clear the buffers.
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}


void D3D::EndScene()
{
    // Present the back buffer to the screen.
    if (m_vSyncEnabled)
    {
        // Lock to screen refresh rate.
        m_swapChain->Present(1, 0);
    }
    else
    {
        m_swapChain->Present(0, 0);
    }

    return;
}


ID3D11Device *D3D::GetDevice()
{
    return m_device;
}


ID3D11DeviceContext *D3D::GetDeviceContext()
{
    return m_deviceContext;
}


void D3D::GetProjectionMatrix(XMMATRIX &projectionMatrix)
{
    projectionMatrix = m_projectionMatrix;
    return;
}


void D3D::GetWorldMatrix(XMMATRIX &worldMatrix)
{
    worldMatrix = m_worldMatrix;
    return;
}


void D3D::GetOrthoMatrix(XMMATRIX &orthoMatrix)
{
    orthoMatrix = m_orthoMatrix;
    return;
}


void D3D::GetVideoCardInfo(char *cardName, int &memory)
{
    strcpy_s(cardName, 128, m_videoCardDescription);
    memory = m_videoCardMemory;
    return;
}


void D3D::TurnZBufferOn()
{
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
    return;
}


void D3D::TurnZBufferOff()
{
    m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
    return;
}


void D3D::ToggleWireframe()
{
    if (!m_wireFrameEnabled)
    {
        SetWireframe(true);
    }
    else
    {
        SetWireframe(false);
    }
}


void D3D::SetWireframe(bool wireframe)
{
    if (wireframe)
    {
        m_deviceContext->RSSetState(m_rasterStateWireFrame);
        m_wireFrameEnabled = true;
    }
    else
    {
        m_deviceContext->RSSetState(m_rasterStateSolid);
        m_wireFrameEnabled = false;
    }
}

void D3D::TurnOnCulling()
{
    m_deviceContext->RSSetState(m_rasterStateSolid);

    return;
}


void D3D::TurnOffCulling()
{
    m_deviceContext->RSSetState(m_rasterStateNoCulling);

    return;
}


void D3D::SetFullscreen(bool fullscreen)
{
    if (fullscreen)
    {
        m_swapChain->SetFullscreenState(true, NULL);
    }
    else
    {
        m_swapChain->SetFullscreenState(false, NULL);
    }
}