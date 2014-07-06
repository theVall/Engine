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

#include "Vec4f.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class ShaderProgram
{
protected:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &);
    virtual ~ShaderProgram();

    bool Initialize(ID3D11Device *, HWND, WCHAR* vsFilename, WCHAR* psFilename);
    bool Initialize(ID3D11Device *, HWND, WCHAR* vsFilename, WCHAR* psFilename, WCHAR* gsFilename);
    void Shutdown();

protected:
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
    // pure virtual methods
    virtual bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) = 0;
    virtual void ShutdownShader() = 0;

protected:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer;
};

