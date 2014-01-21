#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fstream>

using namespace DirectX;
using namespace DirectX::PackedVector;

class ColorShader
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

public:
    ColorShader();
    ColorShader(const ColorShader &);
    ~ColorShader();

    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader*  m_pixelShader;
    ID3D11InputLayout*  m_layout;
    ID3D11Buffer*       m_matrixBuffer;
};

