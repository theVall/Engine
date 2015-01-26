#pragma once

#include <fstream>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

#include "Texture.h"
#include "Vec2f.h"

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class Mandelbrot
{
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
                                ID3D11DeviceContext *pContext);

    ID3D11ShaderResourceView *GetHeightMap();

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
    ID3D11ShaderResourceView *m_pHeightSrv;
    ID3D11UnorderedAccessView *m_pHeightUav;

    //ID3D11Buffer *m_pColorBuffer;
    //ID3D11ShaderResourceView *m_pColorSrv;

    // Shader
    ID3D11ComputeShader *m_pMandelbrotCS;

    // constant buffers
    ID3D11Buffer *m_pImmutableConstBuf;
    ID3D11Buffer *m_pPerFrameConstBuf;
};

