#include "Ocean.h"


Ocean::Ocean()
{
}


Ocean::Ocean(const Ocean &)
{
}


Ocean::~Ocean()
{
}


bool Ocean::Initialize(OceanParameter &params,
                       ID3D11Device *pDevice,
                       ID3D11DeviceContext *pContext,
                       HWND hwnd,
                       WCHAR *vsFilename,
                       WCHAR *psFilename,
                       WCHAR *csFilename)
{
    HRESULT hres;
    m_params = params;

    // height map
    int heightMapDim = params.displacementMapDim;
    int heightMapSize = (heightMapDim + 4) * (heightMapDim + 1);
    XMFLOAT2 *heightData = new XMFLOAT2[heightMapSize * sizeof(XMFLOAT2)];
    float *omegaData = new float[heightMapSize * sizeof(float)];
    InitializeHeightMap(params, heightData, omegaData);

    UINT float2size = 2 * sizeof(float);

    // 2D vector -> scalar, only half the size is needed
    int fftBufferSize = (heightMapDim / 2 + 1) * heightMapDim;

    char *zeroData = new char[fftBufferSize * float2size];
    memset(zeroData, 0, fftBufferSize * float2size);

    // Initial height buffer H(0)
    if (!InitializeBuffer(pDevice,
                          heightData,
                          heightMapSize * float2size,
                          float2size,
                          &m_pBufferFloat2H0,
                          &m_pUavH0,
                          &m_pSrvH0))
    {
        return false;
    }
    // Omega buffer (angular frequencies)
    if (!InitializeBuffer(pDevice,
                          omegaData,
                          heightMapSize * sizeof(float),
                          sizeof(float),
                          &m_pBufferFloatOmega,
                          &m_pUavOmega,
                          &m_pSrvOmega))
    {
        return false;
    }

    // Initialize frequency domain buffers with zeros
    //
    // H(t), height
    if(!InitializeBuffer(pDevice,
                         zeroData,
                         fftBufferSize * float2size,
                         float2size,
                         &m_pBufferFloat2Ht,
                         &m_pUavHt,
                         &m_pSrvHt))
    {
        return false;
    }
    // Dx(t), choppy field
    if (!InitializeBuffer(pDevice, zeroData,
                          fftBufferSize * float2size,
                          float2size,
                          &m_pBufferFloat2Dxt,
                          &m_pUavDxt,
                          &m_pSrvDxt))
    {
        return false;
    }
    // Dy(t), choppy field
    if (!InitializeBuffer(pDevice,
                          zeroData,
                          fftBufferSize * float2size,
                          float2size,
                          &m_pBufferFloat2Dyt,
                          &m_pUavDyt,
                          &m_pSrvDyt))
    {
        return false;
    }

    // Initialize space domain buffers with zeros
    //
    // Dz(t), height
    if (!InitializeBuffer(pDevice,
                          zeroData,
                          fftBufferSize * float2size,
                          float2size,
                          &m_pBufferFloat2Dz,
                          &m_pUavDz,
                          &m_pSrvDz))
    {
        return false;
    }
    // Dx(t), choppy
    if (!InitializeBuffer(pDevice,
                          zeroData,
                          fftBufferSize * float2size,
                          float2size,
                          &m_pBufferFloat2Dx,
                          &m_pUavDx,
                          &m_pSrvDx))
    {
        return false;
    }
    // Dy(t), choppy
    if (!InitializeBuffer(pDevice,
                          zeroData,
                          fftBufferSize * float2size,
                          float2size,
                          &m_pBufferFloat2Dy,
                          &m_pUavDy,
                          &m_pSrvDy))
    {
        return false;
    }

    // cleanup buffer data
    delete(zeroData);
    delete(heightData);
    delete(omegaData);

    // Textures
    //
    // Displacement map
    m_pDisplacementTex = new Texture;
    if (!m_pDisplacementTex)
    {
        return false;
    }
    if (!m_pDisplacementTex->Create2DTextureAndViews(pDevice,
                                                     heightMapDim,
                                                     heightMapDim,
                                                     DXGI_FORMAT_R32G32B32A32_FLOAT))
    {
        return false;
    }
    // Gradient map
    m_pGradientTex = new Texture;
    if (!m_pGradientTex)
    {
        return false;
    }
    if (!m_pGradientTex->Create2DTextureAndViews(pDevice,
                                                 heightMapDim,
                                                 heightMapDim,
                                                 DXGI_FORMAT_R16G16B16A16_FLOAT))
    {
        return false;
    }
    // Sampler
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.MaxLOD = FLT_MAX;
    hres = pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);
    if (FAILED(hres))
    {
        return false;
    }

    // Shaders
    //
    if (!InitializeShader(pDevice, pContext, hwnd, vsFilename, psFilename, csFilename))
    {
        return false;
    }

    // Create FFT
    //
    m_pFft = new FFT();
    if (!m_pFft->Initialize512(pDevice, hwnd, L"../Engine/shader/FftCS.hlsl", 3))
    {
        return false;
    }

    return true;
}

bool Ocean::InitializeShader(ID3D11Device *pDevice,
                             ID3D11DeviceContext *pContext,
                             HWND hwnd,
                             WCHAR *vsFilename,
                             WCHAR *psFilename,
                             WCHAR *csFilename)
{
    HRESULT hres;

    ID3D10Blob *errorMessage = 0;
    ID3D10Blob *pVertexShaderBuffer = 0;
    ID3D10Blob *pDisplacementPSBuffer = 0;
    ID3D10Blob *pGradientPSBuffer = 0;
    ID3D10Blob *pComputeShaderBuffer = 0;

    // Compile shaders
    //
    // Vertex shader (Quad)
    hres = D3DCompileFromFile(vsFilename,
                              NULL,
                              NULL,
                              "Main",
                              "vs_5_0",
                              NULL,
                              NULL,
                              &pVertexShaderBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }
    // Displacement pixel shader
    hres = D3DCompileFromFile(psFilename,
                              NULL,
                              NULL,
                              "UpdateDisplacementPS",
                              "ps_5_0",
                              NULL,
                              NULL,
                              &pDisplacementPSBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }
    // Calculate normals and foldings pixel shader
    hres = D3DCompileFromFile(psFilename,
                              NULL,
                              NULL,
                              "NormalsFoldingsPS",
                              "ps_5_0",
                              NULL,
                              NULL,
                              &pGradientPSBuffer,
                              &errorMessage);
    if (FAILED(hres))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }
    // Compute shader
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

    // Create shader
    //
    hres = pDevice->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(),
                                       pVertexShaderBuffer->GetBufferSize(),
                                       NULL,
                                       &m_pQuadVS);
    if (FAILED(hres))
    {
        return false;
    }
    hres = pDevice->CreatePixelShader(pDisplacementPSBuffer->GetBufferPointer(),
                                      pDisplacementPSBuffer->GetBufferSize(),
                                      NULL,
                                      &m_pUpdateDisplacementPS);
    if (FAILED(hres))
    {
        return false;
    }
    hres = pDevice->CreatePixelShader(pGradientPSBuffer->GetBufferPointer(),
                                      pGradientPSBuffer->GetBufferSize(),
                                      NULL,
                                      &m_pNormalsFoldingsPS);
    if (FAILED(hres))
    {
        return false;
    }
    hres = pDevice->CreateComputeShader(pComputeShaderBuffer->GetBufferPointer(),
                                        pComputeShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pSimulationCS);
    if (FAILED(hres))
    {
        return false;
    }

    // Input layout
    D3D11_INPUT_ELEMENT_DESC vertLayoutDesc[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
    };

    hres = pDevice->CreateInputLayout(vertLayoutDesc,
                                      1,
                                      pVertexShaderBuffer->GetBufferPointer(),
                                      pVertexShaderBuffer->GetBufferSize(),
                                      &m_pVSLayout);
    if (FAILED(hres))
    {
        return false;
    }

    // VS buffer description
    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth = 4 * sizeof(XMFLOAT4);
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;

    float quadVerts[] =
    {
        -1, -1, 0, 1,
        -1,  1, 0, 1,
        1, -1, 0, 1,
        1,  1, 0, 1,
    };
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &quadVerts[0];
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    hres = pDevice->CreateBuffer(&vbDesc, &initData, &m_pQuadVB);
    if (FAILED(hres))
    {
        return false;
    }

    // Constant shader buffers
    //
    UINT displacementDim = m_params.displacementMapDim;
    UINT inputWidth = displacementDim + 4;
    UINT outputWidth = displacementDim / 2 + 1;
    UINT outputHeight = displacementDim;
    UINT constants[] = { displacementDim, inputWidth, outputWidth, outputHeight };

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
    pContext->PSSetConstantBuffers(0, 1, constBuffers);

    // Constants that change on a per frame basis
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.ByteWidth = CalcPad16(sizeof(float) * 3);
    pDevice->CreateBuffer(&cbDesc, NULL, &m_pPerFrameConstBuf);
    if (FAILED(hres))
    {
        return false;
    }

    // cleanup
    pVertexShaderBuffer->Release();
    pVertexShaderBuffer = 0;

    pDisplacementPSBuffer->Release();
    pDisplacementPSBuffer = 0;

    pGradientPSBuffer->Release();
    pGradientPSBuffer = 0;

    pComputeShaderBuffer->Release();
    pComputeShaderBuffer = 0;

    return true;
}


bool Ocean::InitializeBuffer(ID3D11Device *pDevice,
                             void *data,
                             UINT byteWidth,
                             UINT byteStride,
                             ID3D11Buffer **ppBuffer,
                             ID3D11UnorderedAccessView **ppUAV,
                             ID3D11ShaderResourceView **ppSRV)
{
    HRESULT result;

    // Create buffer
    D3D11_BUFFER_DESC bufDesc;
    bufDesc.ByteWidth = byteWidth;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = byteStride;

    D3D11_SUBRESOURCE_DATA initData = { data, 0, 0 };

    result = pDevice->CreateBuffer(&bufDesc, data != NULL ? &initData : NULL, ppBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Create unordered access view
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = byteWidth / byteStride;
    uavDesc.Buffer.Flags = 0;

    result = pDevice->CreateUnorderedAccessView(*ppBuffer, &uavDesc, ppUAV);
    if (FAILED(result))
    {
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = byteWidth / byteStride;

    result = pDevice->CreateShaderResourceView(*ppBuffer, &srvDesc, ppSRV);
    if (FAILED(result))
    {
        return false;
    }
    return true;
}


float Ocean::GenerateGaussRand()
{
    float r1 = rand() / (float)RAND_MAX;
    float r2 = rand() / (float)RAND_MAX;

    if (r1 < 1e-6f)
    {
        r1 = 1e-6f;
    }

    return sqrtf(-2.0f * logf(r1)) * cosf(2.0f * F_PI * r2);
}


float Ocean::GeneratePhillips(Vec2f waveVec,
                              Vec2f windDir,
                              float windVelo,
                              float amplitude,
                              float dirDepend)
{
    // largest possible wave
    float L = windVelo * windVelo / GRAVITY;
    float damping = L / 1000.0f;

    float waveSqr = waveVec.x * waveVec.x + waveVec.y * waveVec.y;
    float omega = waveVec.x * windDir.x + waveVec.y * windDir.y;

    float phillips = amplitude * expf(-1.0f / (L * L * waveSqr)) /
                     (waveSqr * waveSqr * waveSqr) * (omega * omega);

    // filter out waves moving opposite wind
    if (omega < 0)
    {
        phillips *= dirDepend;
    }

    // damp out waves with very small length
    return phillips * expf(-waveSqr * damping * damping);
}


bool Ocean::InitializeHeightMap(OceanParameter &params,
                                XMFLOAT2 *heightData,
                                float *omega)
{
    Vec2f waveVec;
    Vec2f windDir;

    windDir.Normalize();

    float amplitude = params.waveAmplitude * 1e-7f;
    float windVelo = params.windVelo;
    float dirDepend = params.windDependency;

    int heightMapDim = params.displacementMapDim;

    int j;
    int index;
    float phil;

    // Seed random generator
    srand(0);

    // height map must be squared
    for (int i = 0; i < heightMapDim - 1; ++i)
    {
        waveVec.y = (-heightMapDim / 2.0f + i) * (2.0f * F_PI);

        for (j = 0; i < heightMapDim - 1; ++i)
        {
            waveVec.x = (-heightMapDim / 2.0f + j) * (2.0f * F_PI);

            if (waveVec.x == 0.0f && waveVec.y == 0.0f)
            {
                phil = 0.0f;
            }
            else
            {
                phil = sqrtf(GeneratePhillips(waveVec, windDir, windVelo, amplitude, dirDepend));
            }

            index = i * (heightMapDim + 4) + j;

            // initial height H0 (cf. eq. 25 Tessendorf)
            heightData[index].x = float(phil * GenerateGaussRand() * F_HALF_SQRT_2);
            heightData[index].y = float(phil * GenerateGaussRand() * F_HALF_SQRT_2);

            // Angular frequencies: Gerstner wave (cf. eq. 9, 10 Tessendorf)
            omega[index] = sqrtf(GRAVITY * sqrtf(waveVec.x * waveVec.x + waveVec.y * waveVec.y));
        }
    }
    return true;
}


void Ocean::Shutdown()
{
    // Textures
    if (m_pDisplacementTex)
    {
        m_pDisplacementTex->Shutdown();
        m_pDisplacementTex = 0;
    }
    if (m_pGradientTex)
    {
        m_pGradientTex->Shutdown();
        m_pGradientTex = 0;
    }
    m_pPointSampler->Release();

    // Buffers
    ShutdownBuffers();

    // FFT
    if (m_pFft)
    {
        m_pFft->Shutdown512();
        m_pFft = 0;
    }
}


void Ocean::ShutdownBuffers()
{
    m_pBufferFloat2H0->Release();
    m_pUavH0->Release();
    m_pSrvH0->Release();

    m_pBufferFloatOmega->Release();
    m_pUavOmega->Release();
    m_pSrvOmega->Release();

    m_pBufferFloat2Ht->Release();
    m_pUavHt->Release();
    m_pSrvHt->Release();
    m_pBufferFloat2Dxt->Release();
    m_pUavDxt->Release();
    m_pSrvDxt->Release();
    m_pBufferFloat2Dyt->Release();
    m_pUavDyt->Release();
    m_pSrvDyt->Release();

    m_pBufferFloat2Dx->Release();
    m_pUavDx->Release();
    m_pSrvDx->Release();
    m_pBufferFloat2Dy->Release();
    m_pUavDy->Release();
    m_pSrvDy->Release();
    m_pBufferFloat2Dz->Release();
    m_pUavDz->Release();
    m_pSrvDz->Release();

    m_pQuadVB->Release();

    m_pSimulationCS->Release();
    m_pNormalsFoldingsPS->Release();
    m_pUpdateDisplacementPS->Release();
    m_pQuadVS->Release();

    m_pVSLayout->Release();

    m_pPerFrameConstBuf->Release();
    m_pImmutableConstBuf->Release();
}


bool Ocean::UpdateDisplacement(float time, ID3D11DeviceContext *pContext)
{
    HRESULT hres;

    // Calculate data for the time step: H(0) -> H(t), D(x, t), D(y, t)
    pContext->CSSetShader(m_pSimulationCS, NULL, 0);

    // Set H(t) and Omega buffers
    ID3D11ShaderResourceView *cs0Srvs[2] = { m_pSrvH0, m_pSrvOmega };
    pContext->CSSetShaderResources(0, 2, cs0Srvs);
    ID3D11UnorderedAccessView *cs0Uavs[1] = { m_pUavHt };
    pContext->CSSetUnorderedAccessViews(0, 1, cs0Uavs, (UINT *)(&cs0Uavs[0]));

    // Update per frame constants
    D3D11_MAPPED_SUBRESOURCE mappedRes;
    hres = pContext->Map(m_pPerFrameConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
    if (FAILED(hres))
    {
        return false;
    }
    float *perFrameData = (float *)mappedRes.pData;
    perFrameData[0] = time * m_params.timeScale;
    perFrameData[1] = m_params.choppyScale;
    perFrameData[2] = (float)m_params.displacementMapDim; // div by patch length?
    pContext->Unmap(m_pPerFrameConstBuf, 0);
    ID3D11Buffer *csCbs[2] = { m_pImmutableConstBuf, m_pPerFrameConstBuf };
    pContext->CSSetConstantBuffers(0, 2, csCbs);

    // Run the computation
    UINT groupCountX = (m_params.displacementMapDim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
    UINT groupCountY = (m_params.displacementMapDim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
    pContext->Dispatch(groupCountX, groupCountY, 1);
    // Unbind resources
    cs0Uavs[0] = NULL;
    pContext->CSSetUnorderedAccessViews(0, 1, cs0Uavs, (UINT *)(&cs0Uavs[0]));
    cs0Srvs[0] = NULL;
    cs0Srvs[1] = NULL;
    pContext->CSSetShaderResources(0, 2, cs0Srvs);

    // Run FFTs
    //
    m_pFft->Calculate512(m_pUavDx, m_pSrvDx, m_pSrvHt);
    m_pFft->Calculate512(m_pUavDy, m_pSrvDy, m_pSrvHt);
    m_pFft->Calculate512(m_pUavDz, m_pSrvDz, m_pSrvHt);

    // Positions
    //
    // Push old RTV onto 'stack'
    ID3D11RenderTargetView *pOldRtv;
    ID3D11DepthStencilView *pOldDepthStencil;
    pContext->OMGetRenderTargets(1, &pOldRtv, &pOldDepthStencil);
    D3D11_VIEWPORT oldViewPort;
    UINT numViewports = 1;
    pContext->RSGetViewports(&numViewports, &oldViewPort);

    D3D11_VIEWPORT newViewport = { 0,
                                   0,
                                   (float)m_params.displacementMapDim,
                                   (float)m_params.displacementMapDim,
                                   0.0f,
                                   1.0f
                                 };
    pContext->RSSetViewports(1, &newViewport);

    // Set render target view
    ID3D11RenderTargetView *pRtvs[1] = { m_pDisplacementTex->GetRtv() };
    pContext->OMSetRenderTargets(1, pRtvs, NULL);

    // Vertex and pixel shaders
    pContext->VSSetShader(m_pQuadVS, NULL, 0);
    pContext->PSSetShader(m_pUpdateDisplacementPS, NULL, 0);

    // Constant buffers
    ID3D11Buffer *pPSconstBuf[2] = { m_pImmutableConstBuf, m_pPerFrameConstBuf };
    pContext->PSSetConstantBuffers(0, 2, pPSconstBuf);

    // Buffer resources (space domain Dx, Dy, Dz)
    ID3D11ShaderResourceView *pPSsrvs[3] = { m_pSrvDx, m_pSrvDy, m_pSrvDz };
    pContext->PSSetShaderResources(0, 3, pPSsrvs);

    ID3D11Buffer *pVertexBuffers[1] = { m_pQuadVB };
    UINT strides[1] = { sizeof(XMFLOAT4) };
    UINT offsets[1] = { 0 };
    pContext->IASetVertexBuffers(0, 1, &pVertexBuffers[0], &strides[0], &offsets[0]);
    pContext->IASetInputLayout(m_pVSLayout);
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pContext->Draw(4, 0);

    // Unbind pixel SRV
    pPSsrvs[0] = NULL;
    pContext->PSSetShaderResources(0, 1, pPSsrvs);

    // Calculate normals and folding
    //
    // Set render target view
    pRtvs[0] = m_pGradientTex->GetRtv();
    pContext->OMSetRenderTargets(1, pRtvs, NULL);

    // Vertex and pixel shaders
    pContext->VSSetShader(m_pQuadVS, NULL, 0);
    pContext->PSSetShader(m_pNormalsFoldingsPS, NULL, 0);

    // Texture resource from displacement map and point sampler
    pPSsrvs[0] = m_pDisplacementTex->GetSrv();
    pContext->PSSetShaderResources(0, 1, pPSsrvs);
    ID3D11SamplerState *pSamplers[1] = { m_pPointSampler };
    pContext->PSSetSamplers(0, 1, &pSamplers[0]);

    pContext->Draw(4, 0);

    // Unbind pixel SRV
    pPSsrvs[0] = NULL;
    pContext->PSSetShaderResources(0, 1, pPSsrvs);

    // Pop old RTV from 'stack'
    pContext->RSSetViewports(1, &oldViewPort);
    pContext->OMSetRenderTargets(1, &pOldRtv, pOldDepthStencil);
    //pContext->GenerateMips(m_pGradientTex->GetSrv());

    // cleanup
    pOldRtv->Release();
    pOldDepthStencil->Release();

    return true;
}


ID3D11ShaderResourceView *Ocean::GetDisplacementMap()
{
    return m_pDisplacementTex->GetSrv();
}


ID3D11ShaderResourceView *Ocean::GetGradientMap()
{
    return m_pGradientTex->GetSrv();
}


const Ocean::OceanParameter &Ocean::GetParameters()
{
    return m_params;
}


int Ocean::CalcPad16(int n)
{
    return ((n + 15) / 16*16);
}


void Ocean::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename)
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