#pragma once

#include <fstream>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

#include "Texture.h"
#include "Vec2f.h"
#include "Math.h"

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class Mandelbrot
{
    struct PerFrameBufferTypeCS
    {
        float upperLeftX;
        float upperLeftY;
        float lowerRightX;
        float lowerRightY;
        float iterations;
        float maskSize;
        // size for up to 157*4 floats (i.e. a mask of 25*25 is maximum)
        XMFLOAT4 mask[512];
    };

public:
    Mandelbrot();
    Mandelbrot(const Mandelbrot &);
    ~Mandelbrot();

    bool Initialize(ID3D11Device *pDevice,
                    ID3D11DeviceContext *pContext,
                    HWND hwnd,
                    WCHAR *pCsFilename,
                    int heightMapDim);

    void Shutdown();

    // Calculate Mandelbrot set as height data in given rectangle.
    bool CalcHeightsInRectangle(Vec2f upperLeft,
                                Vec2f lowerRight,
                                float iterations,
                                float blurVariance,
                                const UINT maskSize,
                                ID3D11DeviceContext *pContext);

    ID3D11ShaderResourceView *GetHeightMap();
    ID3D11ShaderResourceView *GetHeightTex();


private:

    bool InitializeShader(ID3D11Device *pDevice,
                          ID3D11DeviceContext *pContext,
                          HWND hwnd,
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
    ID3D11Buffer *m_pGaussBuffer;

    ID3D11Texture2D *m_pTex;

    ID3D11UnorderedAccessView *m_pHeightUav;
    ID3D11UnorderedAccessView *m_pGaussUav;
    ID3D11UnorderedAccessView *m_pTexUav;

    ID3D11ShaderResourceView *m_pHeightSrv;
    ID3D11ShaderResourceView *m_pTexSrv;

    // Shader
    ID3D11ComputeShader *m_pMandelbrotCS;
    ID3D11ComputeShader *m_pGaussBlurCS;

    // constant buffers
    ID3D11Buffer *m_pImmutableConstBuf;
    ID3D11Buffer *m_pPerFrameConstBuf;
};

