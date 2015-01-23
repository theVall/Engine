#pragma once

#include <fstream>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

#include "Texture.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Mandelbrot
{
public:
    Mandelbrot();
    Mandelbrot(const Mandelbrot &);
    ~Mandelbrot();

    bool Initialize(ID3D11Device *pDevice,
                    ID3D11DeviceContext *pContext,
                    HWND hwnd,
                    WCHAR *pVsFilename,
                    WCHAR *pPsFilename,
                    WCHAR *pCsFilename,
                    int heightMapDim);

    void Shutdown();

    ID3D11ShaderResourceView *GetDisplacementMap();

private:

    bool InitializeShader(ID3D11Device *pDevice,
                          ID3D11DeviceContext *pContext,
                          HWND hwnd,
                          WCHAR *pVsFilename,
                          WCHAR *pPsFilename,
                          WCHAR *pCsFilename);

    // Shader error output message and log.
    void OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                  HWND hwnd,
                                  WCHAR *shaderFilename);

    template<typename T> void SafeRelease(T *&obj)
    {
        if (obj)
        {
            obj->Release();
            obj = NULL;
        }
    }

    int CalcPad16(int n);

private:

    int m_heightMapDim;

    ID3D11Buffer *m_pHeightBuffer;
    ID3D11UnorderedAccessView *m_pHeightUav;
    ID3D11ShaderResourceView *m_pHeightSrv;

    Texture *m_pDisplacementTex;        // R32F
    ID3D11SamplerState *m_pPointSampler;

    // Shaders, layouts and constants
    ID3D11VertexShader *m_pQuadVS;
    ID3D11PixelShader *m_pDisplacementPS;
    ID3D11ComputeShader *m_pMandelbrotCS;

    ID3D11InputLayout *m_pVSLayout;

    ID3D11Buffer *m_pQuadVB;

    // constant buffers
    ID3D11Buffer *m_pImmutableConstBuf;
    ID3D11Buffer *m_pPerFrameConstBuf;
};

