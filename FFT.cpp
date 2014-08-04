#include "FFT.h"


FFT::FFT()
{
}


FFT::FFT(const FFT &)
{
}


FFT::~FFT()
{
}


bool FFT::Initialize512(ID3D11Device *pDevice, HWND hwnd, WCHAR *csFilename, UINT slices)
{
    m_slices = slices;
    pDevice->GetImmediateContext(&m_pContext);
    if (!m_pContext)
    {
        return false;
    }

    HRESULT hres;

    ID3D10Blob *errorMessage = NULL;
    ID3D10Blob *pComputeShaderBuffer = 0;
    ID3D10Blob *pComputeShaderBuffer2 = 0;

    // Compile compute shaders
    //
    hres = D3DCompileFromFile(csFilename,
                              NULL,
                              NULL,
                              "Main",
                              "cs_5_0",
                              NULL,
                              NULL,
                              &pComputeShaderBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, csFilename);
        }
        else
        {
            MessageBox(hwnd, csFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    hres = pDevice->CreateComputeShader(pComputeShaderBuffer->GetBufferPointer(),
                                        pComputeShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pComputeShader);
    if (FAILED(hres))
    {
        return false;
    }

    // Compile the shader for the last step
    hres = D3DCompileFromFile(csFilename,
                              NULL,
                              NULL,
                              "Radix8_1",
                              "cs_5_0",
                              NULL,
                              NULL,
                              &pComputeShaderBuffer2,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, csFilename);
        }
        else
        {
            MessageBox(hwnd, csFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    hres = pDevice->CreateComputeShader(pComputeShaderBuffer2->GetBufferPointer(),
                                        pComputeShaderBuffer2->GetBufferSize(),
                                        NULL,
                                        &m_pComputeShader2);
    if (FAILED(hres))
    {
        return false;
    }

    // Constant buffers
    //
    D3D11_BUFFER_DESC constBufDesc;
    constBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = 0;
    constBufDesc.MiscFlags = 0;
    constBufDesc.ByteWidth = 32;
    constBufDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA constBufData;
    constBufData.SysMemPitch = 0;
    constBufData.SysMemSlicePitch = 0;

    // Buffer 0: 32768
    const UINT threadCnt = slices*512*512 / 8;
    UINT outStride = 512*512 / 8;
    UINT inStride = outStride;
    double phase = -F_PIMUL2 / (512.0 * 512.0);
    ConstBufStructure constBufData0 = { threadCnt, outStride, inStride, 512, (float)phase };
    constBufData.pSysMem = &constBufData0;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[0]);
    if (!m_pConstBuffer[0])
    {
        return false;
    }

    // Buffer 1: 4096
    inStride /= 8;
    phase *= 8.0;
    ConstBufStructure constBufData1 = { threadCnt, outStride, inStride, 512, (float)phase };
    constBufData.pSysMem = &constBufData1;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[1]);
    if (!m_pConstBuffer[1])
    {
        return false;
    }

    // Buffer 2: 512
    inStride /= 8;
    phase *= 8.0;
    ConstBufStructure constBufData2 = { threadCnt, outStride, inStride, 512, (float)phase };
    constBufData.pSysMem = &constBufData2;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[2]);
    if (!m_pConstBuffer[2])
    {
        return false;
    }

    // Buffer 3: 64
    inStride /= 8;
    phase *= 8.0;
    outStride /= 512;
    ConstBufStructure constBufData3 = { threadCnt, outStride, inStride, 1, (float)phase };
    constBufData.pSysMem = &constBufData3;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[3]);
    if (!m_pConstBuffer[3])
    {
        return false;
    }

    // Buffer 4: 8
    inStride /= 8;
    phase *= 8.0;
    ConstBufStructure constBufData4 = { threadCnt, outStride, inStride, 1, (float)phase };
    constBufData.pSysMem = &constBufData4;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[4]);
    if (!m_pConstBuffer[4])
    {
        return false;
    }

    // Buffer 5: 1
    inStride /= 8;
    phase *= 8.0;
    ConstBufStructure constBufData5 = { threadCnt, outStride, inStride, 1, (float)phase };
    constBufData.pSysMem = &constBufData5;
    pDevice->CreateBuffer(&constBufDesc, &constBufData, &m_pConstBuffer[5]);
    if (!m_pConstBuffer[5])
    {
        return false;
    }

    // Temp buffer, UAV and SRV
    UINT float2size = 2 * sizeof(float);

    D3D11_BUFFER_DESC bufDesc;
    bufDesc.ByteWidth = float2size * 512 * slices * 512;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = float2size;

    pDevice->CreateBuffer(&bufDesc, NULL, &m_pTempBuffer);
    if (!m_pTempBuffer)
    {
        return false;
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = 512 * slices * 512;
    uavDesc.Buffer.Flags = 0;

    pDevice->CreateUnorderedAccessView(m_pTempBuffer, &uavDesc, &m_pTempUav);
    if (!m_pTempUav)
    {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = 512 * slices * 512;

    pDevice->CreateShaderResourceView(m_pTempBuffer, &srvDesc, &m_pTempSrv);
    if (!m_pTempSrv)
    {
        return false;
    }

    // cleanup
    pComputeShaderBuffer->Release();

    return true;
}


bool FFT::Shutdown512()
{
    if (m_pTempSrv)
    {
        m_pTempSrv->Release();
        m_pTempSrv = 0;
    }
    if (m_pTempUav)
    {
        m_pTempUav->Release();
        m_pTempUav = 0;
    }
    if (m_pTempBuffer)
    {
        m_pTempBuffer->Release();
        m_pTempBuffer = 0;
    }
    if (m_pComputeShader)
    {
        m_pComputeShader->Release();
        m_pComputeShader = 0;
    }
    if (m_pContext)
    {
        m_pContext->Release();
        m_pContext = 0;
    }

    for (int i = 0; i < 6; ++i)
    {
        if (m_pConstBuffer[i])
        {
            m_pConstBuffer[i]->Release();
            m_pConstBuffer[i] = 0;
        }
    }

    return true;
}


bool FFT::Calculate512(ID3D11UnorderedAccessView *pUavDst,
                       ID3D11ShaderResourceView *pSrvDst,
                       ID3D11ShaderResourceView *pSrvSrc)
{
    const UINT threadCnt = m_slices * 512 * 512 / 8;
    ID3D11UnorderedAccessView *pTempUav = m_pTempUav;
    ID3D11ShaderResourceView *pTempSrv = m_pTempSrv;
    ID3D11Buffer *constBuffersCS[1];

    UINT inStride = 512 * 512 / 8;
    constBuffersCS[0] = m_pConstBuffer[0];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pTempUav, pSrvSrc, threadCnt, inStride);

    inStride /= 8;
    constBuffersCS[0] = m_pConstBuffer[1];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pUavDst, pTempSrv, threadCnt, inStride);

    inStride /= 8;
    constBuffersCS[0] = m_pConstBuffer[2];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pTempUav, pSrvDst, threadCnt, inStride);

    inStride /= 8;
    constBuffersCS[0] = m_pConstBuffer[3];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pUavDst, pTempSrv, threadCnt, inStride);

    inStride /= 8;
    constBuffersCS[0] = m_pConstBuffer[4];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pTempUav, pSrvDst, threadCnt, inStride);

    inStride /= 8;
    constBuffersCS[0] = m_pConstBuffer[5];
    m_pContext->CSSetConstantBuffers(0, 1, &constBuffersCS[0]);
    Radix8(pUavDst, pTempSrv, threadCnt, inStride);

    return true;
}


void FFT::Radix8(ID3D11UnorderedAccessView *pUavDst,
                 ID3D11ShaderResourceView *pSrvSrc,
                 UINT threadCnt,
                 UINT inStride)
{
    UINT grid = threadCnt / 128;

    ID3D11ShaderResourceView *pSrvs[1] = { pSrvSrc };
    m_pContext->CSSetShaderResources(0, 1, pSrvs);

    ID3D11UnorderedAccessView *pUavs[1] = { pUavDst };
    m_pContext->CSSetUnorderedAccessViews(0, 1, pUavs, (UINT *)(&pUavs[0]));

    if (inStride > 1)
    {
        m_pContext->CSSetShader(m_pComputeShader, NULL, 0);
    }
    else
    {
        m_pContext->CSSetShader(m_pComputeShader2, NULL, 0);
    }

    // Execute
    m_pContext->Dispatch(grid, 1, 1);

    // Unbind
    pSrvs[0] = NULL;
    m_pContext->CSSetShaderResources(0, 1, pSrvs);
    pUavs[0] = NULL;
    m_pContext->CSSetUnorderedAccessViews(0, 1, pUavs, (UINT *)(&pUavs[0]));
}


void FFT::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename)
{
    char *compileErrors;
    unsigned long bufferSize, i;
    std::ofstream fout;

    compileErrors = (char *)(errorMessage->GetBufferPointer());

    bufferSize = errorMessage->GetBufferSize();

    fout.open("ShaderErrorLog.txt");
    for (i = 0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }
    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd,
               L"Error compiling shader. Check ShaderErrorLog.txt for message.",
               shaderFilename,
               MB_OK);

    return;
}