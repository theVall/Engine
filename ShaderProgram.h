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

#include "Vec2f.h"
#include "Vec3f.h"
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

    bool Initialize(ID3D11Device *pDevice,
                    HWND hwnd,
                    WCHAR *vsFilename,
                    WCHAR *psFilename);
    bool Initialize(ID3D11Device *pDevice,
                    HWND hwnd,
                    WCHAR *vsFilename,
                    WCHAR *psFilename,
                    WCHAR *gsFilename);
    bool Initialize(ID3D11Device *pDevice,
                    HWND hwnd,
                    WCHAR *vsFilename,
                    WCHAR *hsFilename,
                    WCHAR *dsFilename,
                    WCHAR *psFilename);

    void Shutdown();

protected:
    void OutputShaderErrorMessage(ID3D10Blob *pBlob, HWND hwnd, WCHAR *pError);

    bool CompileShader(WCHAR *shaderFilename,
                       LPCSTR pEntryPoint,
                       LPCSTR pTarget,
                       ID3DBlob **pCode,
                       HWND hwnd);

    // virtual methods
    virtual bool InitializeShader(ID3D11Device *pDevice,
                                  HWND hwnd,
                                  WCHAR *vsFilename,
                                  WCHAR *psFilename);
    virtual bool InitializeShader(ID3D11Device *pDevice,
                                  HWND hwnd,
                                  WCHAR *vsFilename,
                                  WCHAR *hsFilename,
                                  WCHAR *dsFilename,
                                  WCHAR *psFilename);

    virtual void ShutdownShader() = 0;

    template<typename T> void SafeRelease(T *&obj)
    {
        if (obj)
        {
            obj->Release();
            obj = NULL;
        }
    }

private:
    bool SetRasterStates(ID3D11Device *pDevice);

protected:
    // Shader pointer
    ID3D11VertexShader *m_pVertexShader;
    ID3D11HullShader *m_pHullShader;
    ID3D11DomainShader *m_pDomainShader;
    ID3D11GeometryShader *m_pGeometryShader;
    ID3D11PixelShader *m_pPixelShader;

    ID3D11InputLayout *m_pLayout;
    ID3D11SamplerState *m_pSamplerState;

    ID3D11Buffer *m_pMatrixBuffer;

    ID3D11RasterizerState *m_pRsStateWireframe;
    ID3D11RasterizerState *m_pRsStateSolid;
};

