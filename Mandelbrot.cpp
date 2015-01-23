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
                            WCHAR *pVsFilename,
                            WCHAR *pPsFilename,
                            WCHAR *pCsFilename,
                            int heightMapDim)
{
    HRESULT result;
    m_heightMapDim = heightMapDim;
    int heightMapSize = heightMapDim * heightMapDim;

    float *heightData = new float[heightMapSize];
    for (size_t i = 0; i < heightMapSize; ++i)
    {
        heightData[i] = 0.0f;
    }


    // Create buffer
    D3D11_BUFFER_DESC bufDesc;
    bufDesc.ByteWidth = sizeof(float);
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = heightMapSize * sizeof(float);

    D3D11_SUBRESOURCE_DATA initData = { heightData, 0, 0 };

    result = pDevice->CreateBuffer(&bufDesc,
                                   heightData != NULL ? &initData : NULL,
                                   &m_pHeightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Create unordered access view
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

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = bufDesc.ByteWidth / bufDesc.StructureByteStride;

    result = pDevice->CreateShaderResourceView(m_pHeightBuffer, &srvDesc, &m_pHeightSrv);
    if (FAILED(result))
    {
        return false;
    }

    // clean up
    delete(heightData);

    // Texture
    m_pDisplacementTex = new Texture;
    if (!m_pDisplacementTex)
    {
        return false;
    }
    if (!m_pDisplacementTex->Create2DTextureAndViews(pDevice,
                                                     heightMapDim,
                                                     heightMapDim,
                                                     DXGI_FORMAT_R32_FLOAT))
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
    result = pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);
    if (FAILED(result))
    {
        return false;
    }

    // Shaders
    //
    if (!InitializeShader(pDevice, pContext, hwnd, pVsFilename, pPsFilename, pCsFilename))
    {
        return false;
    }

    return true;
}


void Mandelbrot::Shutdown()
{
    if (m_pDisplacementTex)
    {
        m_pDisplacementTex->Shutdown();
        m_pDisplacementTex = 0;
    }

    m_pPointSampler->Release();

    m_pHeightBuffer->Release();
    m_pHeightUav->Release();
    m_pHeightSrv->Release();
}


ID3D11ShaderResourceView *Mandelbrot::GetDisplacementMap()
{
    return m_pDisplacementTex->GetSrv();
}


bool Mandelbrot::InitializeShader(ID3D11Device *pDevice,
                                  ID3D11DeviceContext *pContext,
                                  HWND hwnd,
                                  WCHAR *pVsFilename,
                                  WCHAR *pPsFilename,
                                  WCHAR *pCsFilename)
{
    HRESULT hres;

    ID3D10Blob *errorMessage = 0;
    ID3D10Blob *pVertexShaderBuffer = 0;
    ID3D10Blob *pDisplacementPSBuffer = 0;
    ID3D10Blob *pComputeShaderBuffer = 0;

    // Compile shaders
    //
    // Vertex shader (Quad)
    hres = D3DCompileFromFile(pVsFilename,
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
            OutputShaderErrorMessage(errorMessage, hwnd, pVsFilename);
        }
        else
        {
            MessageBox(hwnd, pVsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }
    // Displacement pixel shader
    hres = D3DCompileFromFile(pPsFilename,
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
            OutputShaderErrorMessage(errorMessage, hwnd, pPsFilename);
        }
        else
        {
            MessageBox(hwnd, pPsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }
    // Compute shader
    hres = D3DCompileFromFile(pCsFilename,
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
            OutputShaderErrorMessage(errorMessage, hwnd, pCsFilename);
        }
        else
        {
            MessageBox(hwnd, pCsFilename, L"Missing Shader File", MB_OK);
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
                                      &m_pDisplacementPS);
    if (FAILED(hres))
    {
        return false;
    }
    hres = pDevice->CreateComputeShader(pComputeShaderBuffer->GetBufferPointer(),
                                        pComputeShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pMandelbrotCS);
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
        -1, 1, 0, 1,
        1, -1, 0, 1,
        1, 1, 0, 1,
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
    // TODO: CONSTANTS
    UINT displacementDim = m_heightMapDim;
    UINT inputWidth = displacementDim + 4;
    UINT outputWidth = displacementDim;
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
    // TODO: USED?
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.ByteWidth = CalcPad16(sizeof(float));
    pDevice->CreateBuffer(&cbDesc, NULL, &m_pPerFrameConstBuf);
    if (FAILED(hres))
    {
        return false;
    }

    // cleanup
    SafeRelease(pVertexShaderBuffer);
    SafeRelease(pDisplacementPSBuffer);
    SafeRelease(pComputeShaderBuffer);

    return true;
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