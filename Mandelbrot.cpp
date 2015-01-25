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

    // Create SRV for height output
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

    //// Create color buffer
    //bufDesc.ByteWidth = sizeof(float);
    //bufDesc.Usage = D3D11_USAGE_DEFAULT;
    //bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    //bufDesc.CPUAccessFlags = 0;
    //bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    //// 3 times for RGB
    //bufDesc.StructureByteStride = heightMapSize * sizeof(float) * 3;
    //result = pDevice->CreateBuffer(&bufDesc,
    //                               heightData != NULL ? &initData : NULL,
    //                               &m_pColorBuffer);
    //if (FAILED(result))
    //{
    //    return false;
    //}

    //// Create shader resource view for color
    //uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    //uavDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    //uavDesc.Buffer.FirstElement = 0;
    //// 3 times for RGB
    //uavDesc.Buffer.NumElements = (bufDesc.ByteWidth / bufDesc.StructureByteStride) * 3;

    //result = pDevice->CreateShaderResourceView(m_pColorBuffer, &uavDesc, &m_pColorSrv);
    //if (FAILED(result))
    //{
    //    return false;
    //}

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
    m_pHeightSrv->Release();
}


bool Mandelbrot::InitializeShader(ID3D11Device *pDevice,
                                  ID3D11DeviceContext *pContext,
                                  HWND hwnd,
                                  WCHAR *pCsFilename)
{
    HRESULT hres;

    ID3D10Blob *errorMessage = 0;
    ID3D10Blob *pComputeShaderBuffer = 0;

    // Compile compute shader
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
    hres = pDevice->CreateComputeShader(pComputeShaderBuffer->GetBufferPointer(),
                                        pComputeShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pMandelbrotCS);
    if (FAILED(hres))
    {
        return false;
    }


    // Constant shader buffers
    // TODO: CONSTANTS
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
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    // 2 2D coordinates and the number of iterations
    cbDesc.ByteWidth = CalcPad16(sizeof(UINT)*5);
    pDevice->CreateBuffer(&cbDesc, NULL, &m_pPerFrameConstBuf);
    if (FAILED(hres))
    {
        return false;
    }

    // cleanup
    SafeRelease(pComputeShaderBuffer);

    return true;
}


bool Mandelbrot::CalcHeightsInRectangle(UINT upperLeftX,
                                        UINT upperLeftY,
                                        UINT lowerRightX,
                                        UINT lowerRightY,
                                        UINT iterations,
                                        ID3D11DeviceContext *pContext)
{
    HRESULT hres;

    // set up compute shader
    //
    pContext->CSSetShader(m_pMandelbrotCS, NULL, 0);

    // Set output buffers
    ID3D11ShaderResourceView *cs0Srv[1] = { m_pHeightSrv };// , m_pColorSrv };
    pContext->CSSetShaderResources(0, 1, cs0Srv);

    // set coordinates and iterations in per frame constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedRes;
    hres = pContext->Map(m_pPerFrameConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
    if (FAILED(hres))
    {
        return false;
    }
    UINT *perFrameData = (UINT *)mappedRes.pData;
    perFrameData[0] = upperLeftX;
    perFrameData[1] = upperLeftY;
    perFrameData[2] = lowerRightX;
    perFrameData[3] = lowerRightY;
    perFrameData[4] = iterations;
    pContext->Unmap(m_pPerFrameConstBuf, 0);
    ID3D11Buffer *csCbs[2] = { m_pImmutableConstBuf, m_pPerFrameConstBuf };
    pContext->CSSetConstantBuffers(0, 2, csCbs);

    // run compute shader
    UINT groupCountX = (m_heightMapDim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
    UINT groupCountY = (m_heightMapDim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
    pContext->Dispatch(groupCountX, groupCountY, 1);

    // unbind resources
    cs0Srv[0] = NULL;
    //cs0Srvs[1] = NULL;
    pContext->CSSetShaderResources(0, 1, cs0Srv);

    return true;
}


ID3D11ShaderResourceView *Mandelbrot::GetHeightMap()
{
    return m_pHeightSrv;
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