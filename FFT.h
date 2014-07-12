#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

#include <fstream>

#include "math.h"

using namespace math;

class FFT
{
public:
    struct ConstBufStructure
    {
        UINT threadCnt;
        UINT inStride;
        UINT outStride;
        UINT phaseStride;
        float phase;
    };

public:
    FFT();
    FFT(const FFT &);
    ~FFT();

    bool Initialize512(ID3D11Device *pDevice, HWND hwnd, WCHAR *csFilename, UINT slices);

    bool Shutdown512();

    bool Calculate512(ID3D11UnorderedAccessView *pUavDst,
                      ID3D11ShaderResourceView *pSrvDst,
                      ID3D11ShaderResourceView *pSrvSrc);

private:
    ID3D11DeviceContext *m_pContext;
    ID3D11ComputeShader *m_pComputeShader;
    ID3D11ComputeShader *m_pComputeShader2;
    ID3D11Buffer *m_pConstBuffer[6];
    ID3D11Buffer *m_pTempBuffer;
    ID3D11UnorderedAccessView *m_pTempUav;
    ID3D11ShaderResourceView *m_pTempSrv;
    UINT m_slices;

private:
    void Radix8(ID3D11UnorderedAccessView *pUavDst,
                ID3D11ShaderResourceView *pSrvSrc,
                UINT threadCnt,
                UINT inStride);

    void OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                  HWND hwnd,
                                  WCHAR *shaderFilename);

};

