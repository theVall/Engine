#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fstream>
#include <d3dcompiler.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class LightShader
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    struct LightBufferType
    {
        XMFLOAT4 diffuseColor;
        XMFLOAT3 lightDirection;
        // Extra padding so structure is a multiple of 16 for CreateBuffer function.
        float padding;  
    };

public:
    LightShader();
    LightShader(const LightShader &);
    ~LightShader();

    bool Initialize(ID3D11Device *, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext *,
                int,
                const XMMATRIX &,
                const XMMATRIX &,
                const XMMATRIX &,
                ID3D11ShaderResourceView *,
                const XMFLOAT3 &,
                const XMFLOAT4 &);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    bool SetShaderParameters(ID3D11DeviceContext*,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             ID3D11ShaderResourceView *,
                             const XMFLOAT3 &,
                             const XMFLOAT4 &);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_lightBuffer;
};