#include "Mandelbrot.h"


Mandelbrot::Mandelbrot()
{
    m_heightMapDim = 0;
}


Mandelbrot::Mandelbrot(const Mandelbrot &)
{
}


Mandelbrot::~Mandelbrot()
{
}


bool Mandelbrot::Initialize(ID3D11Device *pDevice,
                            ID3D11DeviceContext *pContext,
                            HWND hwnd,
                            WCHAR *pCsFilename,
                            int heightMapDim)
{
    HRESULT result;
    m_heightMapDim = heightMapDim;
    int heightMapSize = heightMapDim * heightMapDim;

    char *zeroDataHeight = new char[heightMapSize * sizeof(float)];
    memset(zeroDataHeight, 0, heightMapSize * sizeof(float));

    // Create height buffer
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(float) * heightMapSize;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(float);

    D3D11_SUBRESOURCE_DATA initData = { zeroDataHeight, 0, 0 };
    result = pDevice->CreateBuffer(&bufDesc,
                                   zeroDataHeight != NULL ? &initData : NULL,
                                   &m_pHeightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Create UAV for height output
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = bufDesc.ByteWidth / bufDesc.StructureByteStride;
    uavDesc.Buffer.Flags = 0;

    result = pDevice->CreateUnorderedAccessView(m_pHeightBuffer, &uavDesc, &m_pHeightUav);
    if (FAILED(result))
    {
        return false;
    }

    // Create UAV for Gauss filtered output
    //
    // Note: Bind buffer as UAV as well as SRV.
    // Important, so that we can write to it from CS and read from it from VS/PS.
    // Use the Gaussian blurred output as SRV because we want that in our VS.
    //
    // exactly the same description as the height buffer
    result = pDevice->CreateBuffer(&bufDesc,
                                   zeroDataHeight != NULL ? &initData : NULL,
                                   &m_pGaussBuffer);
    if (FAILED(result))
    {
        return false;
    }
    // and same UAV description
    result = pDevice->CreateUnorderedAccessView(m_pGaussBuffer, &uavDesc, &m_pGaussUav);
    if (FAILED(result))
    {
        return false;
    }

    // Create SRV for filtered height output (view on same buffer as UAV!)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = bufDesc.ByteWidth / bufDesc.StructureByteStride;

    result = pDevice->CreateShaderResourceView(m_pGaussBuffer, &srvDesc, &m_pHeightSrv);
    if (FAILED(result))
    {
        return false;
    }

    // Create texture resource for texture output (used for minimap)
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.ArraySize = 1;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texDesc.CPUAccessFlags = 0;
    texDesc.Format = DXGI_FORMAT_R16G16_UNORM;
    texDesc.Width = heightMapDim;
    texDesc.Height = heightMapDim;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = 0;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;

    result = pDevice->CreateTexture2D(&texDesc,
                                      NULL,
                                      &m_pTex);
    if (FAILED(result))
    {
        return false;
    }

    // Create UAV for height to texture output
    uavDesc.Format = DXGI_FORMAT_R16G16_UNORM;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    result = pDevice->CreateUnorderedAccessView(m_pTex, &uavDesc, &m_pTexUav);
    if (FAILED(result))
    {
        return false;
    }

    // finally create SRV for the texture resource so that we can access it from PS
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Format = DXGI_FORMAT_R16G16_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    result = pDevice->CreateShaderResourceView(m_pTex, &srvDesc, &m_pTexSrv);
    if (FAILED(result))
    {
        return false;
    }

    // clean up
    delete(zeroDataHeight);

    // Shaders
    //
    if (!InitializeShader(pDevice, pContext, hwnd, pCsFilename))
    {
        return false;
    }

    return true;
}


void Mandelbrot::Shutdown()
{
    m_pHeightBuffer->Release();
    m_pGaussBuffer->Release();
    m_pTex->Release();

    m_pHeightSrv->Release();
    m_pTexSrv->Release();
    m_pHeightUav->Release();
    m_pGaussUav->Release();
    m_pTexUav->Release();

    m_pImmutableConstBuf->Release();
    m_pPerFrameConstBuf->Release();

    m_pMandelbrotCS->Release();
    m_pGaussBlurCS->Release();
}


bool Mandelbrot::InitializeShader(ID3D11Device *pDevice,
                                  ID3D11DeviceContext *pContext,
                                  HWND hwnd,
                                  WCHAR *pCsFilename)
{
    HRESULT hres;

    ID3D10Blob *errorMessage = 0;
    ID3D10Blob *pMandelbrotCSBuffer = 0;
    ID3D10Blob *pGaussCSBuffer = 0;

    // Compile Mandelbrot generation compute shader
    hres = D3DCompileFromFile(pCsFilename,
                              NULL,
                              NULL,
                              "Main",
                              "cs_5_0",
                              NULL,
                              NULL,
                              &pMandelbrotCSBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, pCsFilename);
        }
        else
        {
            MessageBox(hwnd, pCsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Create shader
    hres = pDevice->CreateComputeShader(pMandelbrotCSBuffer->GetBufferPointer(),
                                        pMandelbrotCSBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pMandelbrotCS);
    if (FAILED(hres))
    {
        return false;
    }

    // Compile Gaussian blur compute shader
    hres = D3DCompileFromFile(pCsFilename,
                              NULL,
                              NULL,
                              "GaussBlur",
                              "cs_5_0",
                              NULL,
                              NULL,
                              &pGaussCSBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, pCsFilename);
        }
        else
        {
            MessageBox(hwnd, pCsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Create shader
    hres = pDevice->CreateComputeShader(pGaussCSBuffer->GetBufferPointer(),
                                        pGaussCSBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pGaussBlurCS);
    if (FAILED(hres))
    {
        return false;
    }

    // Constant shader buffers
    UINT displacementDim = m_heightMapDim;
    UINT constants[] = { displacementDim };

    D3D11_SUBRESOURCE_DATA initConstBuf = { &constants[0], 0, 0 };

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.ByteWidth = CalcPad16(sizeof(constants));
    hres = pDevice->CreateBuffer(&cbDesc, &initConstBuf, &m_pImmutableConstBuf);
    if (FAILED(hres))
    {
        return false;
    }

    ID3D11Buffer *constBuffers[1] = { m_pImmutableConstBuf };
    pContext->CSSetConstantBuffers(0, 1, constBuffers);

    // Constants that change on a per frame basis
    D3D11_BUFFER_DESC perFameBufferDesc;
    perFameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    perFameBufferDesc.ByteWidth = CalcPad16(sizeof(PerFrameBufferTypeCS));
    perFameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    perFameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    perFameBufferDesc.MiscFlags = 0;
    perFameBufferDesc.StructureByteStride = 0;

    pDevice->CreateBuffer(&perFameBufferDesc, NULL, &m_pPerFrameConstBuf);
    if (FAILED(hres))
    {
        return false;
    }

    // cleanup
    SafeRelease(pMandelbrotCSBuffer);
    SafeRelease(pGaussCSBuffer);

    return true;
}


bool Mandelbrot::CalcHeightsInRectangle(Vec2f upperLeft,
                                        Vec2f lowerRight,
                                        float iterations,
                                        float blurVariance,
                                        const UINT maskSize,
                                        ID3D11DeviceContext *pContext)
{
    HRESULT hres;

    // Gaussian blur parameters
    float *gaussMask;
    gaussMask = (float *)malloc(sizeof(float)*maskSize*maskSize);
    Math::GetGaussianBlurMask(maskSize, blurVariance, gaussMask);

    // set up Mandelbrot compute shader
    //
    pContext->CSSetShader(m_pMandelbrotCS, NULL, 0);

    // Set output buffers
    ID3D11UnorderedAccessView *cs0Uavs[3] = { m_pHeightUav, m_pGaussUav, m_pTexUav };
    pContext->CSSetUnorderedAccessViews(0, 3, cs0Uavs, (UINT *)(&cs0Uavs[0]));

    // set coordinates and iterations in per frame constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedRes;
    hres = pContext->Map(m_pPerFrameConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
    if (FAILED(hres))
    {
        return false;
    }
    PerFrameBufferTypeCS *perFrameData = (PerFrameBufferTypeCS *)mappedRes.pData;
    perFrameData->upperLeftX = upperLeft.x;
    perFrameData->upperLeftY = upperLeft.y;
    perFrameData->lowerRightX = lowerRight.x;
    perFrameData->lowerRightY = lowerRight.y;
    perFrameData->iterations = iterations;
    perFrameData->maskSize = (float)maskSize;

    for (UINT i = 0; i < maskSize*maskSize; )
    {
        perFrameData->mask[i / 4] = XMFLOAT4(gaussMask[i++], gaussMask[i++],
                                             gaussMask[i++], gaussMask[i++]);
    }

    pContext->Unmap(m_pPerFrameConstBuf, 0);
    ID3D11Buffer *csCbs[2] = { m_pImmutableConstBuf, m_pPerFrameConstBuf };
    pContext->CSSetConstantBuffers(0, 2, csCbs);

    // run compute shader
    UINT groupCountX = (m_heightMapDim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
    UINT groupCountY = (m_heightMapDim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
    pContext->Dispatch(groupCountX, groupCountY, 1);

    // Gauss blur
    //
    // Buffers and constants already set/bound from before, so just dispatch.
    //
    pContext->CSSetShader(m_pGaussBlurCS, NULL, 0);
    pContext->Dispatch(groupCountX, groupCountY, 1);

    // unbind resources
    cs0Uavs[0] = NULL;
    cs0Uavs[1] = NULL;
    cs0Uavs[2] = NULL;
    pContext->CSSetUnorderedAccessViews(0, 3, cs0Uavs, (UINT *)(&cs0Uavs[0]));

    return true;
}


ID3D11ShaderResourceView *Mandelbrot::GetHeightMap()
{
    return m_pHeightSrv;
}


ID3D11ShaderResourceView *Mandelbrot::GetHeightTex()
{
    return m_pTexSrv;
}


int Mandelbrot::CalcPad16(int n)
{
    return ((n + 15) / 16 * 16);
}


void Mandelbrot::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                          HWND hwnd,
                                          WCHAR *shaderFilename)
{
    char *compileErrors;
    unsigned long bufferSize, i;
    ofstream fout;

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