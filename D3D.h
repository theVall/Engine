#pragma once

//  linking
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

//  includes
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class D3D
{
public:

    D3D(void);
    D3D(const D3D &);
    ~D3D(void);

    bool Initialize(int, int, bool, HWND, bool, float, float);

    void Shutdown();

    //  Called on the beginning of each frame. Initializes buffers for drawing.
    void BeginScene(float, float, float, float);

    //  Tells the swap chain to display scene after drawing.
    void EndScene();

    //  Getter functions.
    ID3D11Device * GetDevice();
    ID3D11DeviceContext * GetDeviceContext();

    void GetProjectionMatrix(XMMATRIX &);
    void GetWorldMatrix(XMMATRIX &);
    void GetOrthoMatrix(XMMATRIX &);

    void GetVideoCardInfo(char *, int &);

    void TurnZBufferOn();
    void TurnZBufferOff();

    void ToggleWireframe();

    void TurnOnCulling();
    void TurnOffCulling();

private:

    bool m_vSyncEnabled;
    bool m_wireFrameEnabled;

    int m_videoCardMemory;
    char m_videoCardDescription[128];

    IDXGISwapChain          *m_swapChain;
    ID3D11Device            *m_device;
    ID3D11DeviceContext     *m_deviceContext;
    ID3D11RenderTargetView  *m_renderTargetView;
    ID3D11Texture2D         *m_depthStencilBuffer;
    ID3D11DepthStencilState *m_depthStencilState;
    ID3D11DepthStencilView  *m_depthStencilView;
    ID3D11DepthStencilState *m_depthDisabledStencilState;

    ID3D11RasterizerState *m_rasterStateSolid;
    ID3D11RasterizerState *m_rasterStateWireFrame;
    ID3D11RasterizerState *m_rasterStateNoCulling;

    XMMATRIX m_projectionMatrix;
    XMMATRIX m_worldMatrix;
    XMMATRIX m_orthoMatrix;
};

