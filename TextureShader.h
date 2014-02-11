#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fstream>
#include <d3dcompiler.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class TextureShader
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

public:
    TextureShader();
    TextureShader(const TextureShader &);
    ~TextureShader();

    bool Initialize(ID3D11Device *, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext *,
                int,
                const XMMATRIX &,
                const XMMATRIX &,
                const XMMATRIX &,
                ID3D11ShaderResourceView *);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
    bool SetShaderParameters(ID3D11DeviceContext*,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             ID3D11ShaderResourceView *);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader*  m_pixelShader;
    ID3D11InputLayout*  m_layout;
    ID3D11Buffer*       m_matrixBuffer;
    ID3D11SamplerState* m_sampleState;
};


