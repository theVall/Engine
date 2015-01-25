#include "MandelbrotShader.h"


MandelbrotShader::MandelbrotShader(int meshDim)
{
    m_meshDim = meshDim;
}


MandelbrotShader::MandelbrotShader(const MandelbrotShader &)
{
}


MandelbrotShader::~MandelbrotShader()
{
}


bool MandelbrotShader::InitializeShader(ID3D11Device *pDevice,
                                        HWND hwnd,
                                        WCHAR *vsFilename,
                                        WCHAR *psFilename)
{
    HRESULT result;

    D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
    unsigned int numElements;

    // D3D buffers
    if (!CreateSurfaceVertices(pDevice))
    {
        MessageBox(hwnd, L"Error", L"Error creating ocean surface vertices.", MB_OK);
        return false;
    }

    // Shader
    ID3D10Blob *pError = NULL;
    ID3D10Blob *pBufferVS = NULL;
    ID3D10Blob *pBufferPS = NULL;

    // Compile the vertex shader code.
    result = D3DCompileFromFile(vsFilename,
                                NULL,
                                NULL,
                                "Main",
                                "vs_5_0",
                                NULL,
                                NULL,
                                &pBufferVS,
                                &pError);
    if (FAILED(result))
    {
        if (pError)
        {
            OutputShaderErrorMessage(pError, hwnd, vsFilename);
        }
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Compile the pixel shader code.
    result = D3DCompileFromFile(psFilename,
                                NULL,
                                NULL,
                                "Main",
                                "ps_5_0",
                                NULL,
                                NULL,
                                &pBufferPS,
                                &pError);
    if (FAILED(result))
    {
        if (pError)
        {
            OutputShaderErrorMessage(pError, hwnd, psFilename);
        }
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Create the vertex shader.
    result = pDevice->CreateVertexShader(pBufferVS->GetBufferPointer(),
                                         pBufferVS->GetBufferSize(),
                                         NULL,
                                         &m_pMandelbrotVS);
    if (FAILED(result))
    {
        return false;
    }

    // Create the pixel shaders.
    result = pDevice->CreatePixelShader(pBufferPS->GetBufferPointer(),
                                        pBufferPS->GetBufferSize(),
                                        NULL,
                                        &m_pMandelbrotPS);
    if (FAILED(result))
    {
        return false;
    }

    // Vertex input layout
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    result = pDevice->CreateInputLayout(polygonLayout,
                                        numElements,
                                        pBufferVS->GetBufferPointer(),
                                        pBufferVS->GetBufferSize(),
                                        &m_pLayout);
    if (FAILED(result))
    {
        return false;
    }

    // clean up shader buffers
    SafeRelease(pBufferVS);
    SafeRelease(pBufferPS);

    // Constant buffer
    // Constants
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Per frame buffer for pixel shader
    D3D11_BUFFER_DESC perFameBufferDescPS;
    perFameBufferDescPS.Usage = D3D11_USAGE_DYNAMIC;
    perFameBufferDescPS.ByteWidth = ((sizeof(PerFrameBufferTypePS) + 15) / 16 * 16);
    perFameBufferDescPS.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    perFameBufferDescPS.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    perFameBufferDescPS.MiscFlags = 0;
    perFameBufferDescPS.StructureByteStride = 0;

    result = pDevice->CreateBuffer(&perFameBufferDescPS, NULL, &m_perFameBufferPS);
    if (FAILED(result))
    {
        return false;
    }

    // Samplers
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = FLT_MAX;

    result = pDevice->CreateSamplerState(&samplerDesc, &m_pHeightSampler);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void MandelbrotShader::ShutdownShader()
{
    // D3D Buffer
    SafeRelease(m_pMeshIB);
    SafeRelease(m_pMeshVB);
    SafeRelease(m_pMatrixBuffer);
    //SafeRelease(m_perFameBufferVS);
    SafeRelease(m_perFameBufferPS);
    // Shader
    SafeRelease(m_pMandelbrotVS);
    SafeRelease(m_pMandelbrotPS);
}


bool MandelbrotShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                           const XMMATRIX &worldMatrix,
                                           const XMMATRIX &viewMatrix,
                                           const XMMATRIX &projectionMatrix,
                                           const XMFLOAT3 &lightDir,
                                           ID3D11ShaderResourceView *pHeightSrv)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *pTransformDataBuffer;

    // Vertex shader buffer
    result = pContext->Map(m_pMatrixBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Transformation matrices
    pTransformDataBuffer = (MatrixBufferType *)mappedResource.pData;

    pTransformDataBuffer->world = XMMatrixTranspose(worldMatrix);
    pTransformDataBuffer->view = XMMatrixTranspose(viewMatrix);
    pTransformDataBuffer->projection = XMMatrixTranspose(projectionMatrix);

    pContext->Unmap(m_pMatrixBuffer, 0);

    // Pixel shader constant buffer
    result = pContext->Map(m_perFameBufferPS,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    PerFrameBufferTypePS *pPerFrameDataBufferPS = (PerFrameBufferTypePS *)mappedResource.pData;
    pPerFrameDataBufferPS->lightDir = lightDir;
    pContext->Unmap(m_perFameBufferPS, 0);
    pContext->PSSetConstantBuffers(1, 1, &m_perFameBufferPS);

    ID3D11ShaderResourceView *pSrvs[1] = { pHeightSrv };
    pContext->VSSetShaderResources(0, 1, &pSrvs[0]);

    return true;
}


bool MandelbrotShader::Render(ID3D11DeviceContext *pContext,
                              const XMMATRIX &worldMatrix,
                              const XMMATRIX &viewMatrix,
                              const XMMATRIX &projectionMatrix,
                              const XMFLOAT3 &lightDir,
                              ID3D11ShaderResourceView *pHeightSrv,
                              bool wireframe)
{
    if (!SetShaderParameters(pContext,
                             worldMatrix,
                             viewMatrix,
                             projectionMatrix,
                             lightDir,
                             pHeightSrv))
    {
        return false;
    }

    // Set the vertex and index buffers to active in the input assembler.
    unsigned int stride = sizeof(TerrainVertex);
    unsigned int offset = 0;

    pContext->IASetVertexBuffers(0, 1, &m_pMeshVB, &stride, &offset);
    pContext->IASetIndexBuffer(m_pMeshIB, DXGI_FORMAT_R32_UINT, 0);

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Render
    RenderShader(pContext, wireframe);

    return true;
}


void MandelbrotShader::RenderShader(ID3D11DeviceContext *pContext, bool wireframe)
{
    // Set the vertex input layout.
    pContext->IASetInputLayout(m_pLayout);

    // Set the vertex and pixel shaders.
    pContext->VSSetShader(m_pMandelbrotVS, NULL, 0);
    if (wireframe)
    {
        pContext->RSSetState(m_pRsStateWireframe);
    }
    pContext->PSSetShader(m_pMandelbrotPS, NULL, 0);

    // Sampler
    ID3D11SamplerState *vsSamplers[3] = { m_pHeightSampler };
    pContext->VSSetSamplers(0, 3, &vsSamplers[0]);

    // draw call
    pContext->DrawIndexed(m_numIndices, 0, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView *pNullSrvs[2] = { NULL, NULL };
    pContext->VSSetShaderResources(0, 2, pNullSrvs);
    pContext->PSSetShaderResources(0, 2, pNullSrvs);

    return;
}


bool MandelbrotShader::CreateSurfaceVertices(ID3D11Device *pDevice)
{
    HRESULT hres;

    // Vertex buffer
    int numVertices = (m_meshDim + 1) * (m_meshDim + 1);
    TerrainVertex *pVertices = new TerrainVertex[numVertices];

    for (int i = 0; i <= m_meshDim; i++)
    {
        for (int j = 0; j <= m_meshDim; j++)
        {
            pVertices[i * (m_meshDim + 1) + j].indexX = (float)j;
            pVertices[i * (m_meshDim + 1) + j].indexY = (float)i;
        }
    }

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth = numVertices * sizeof(TerrainVertex);
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = sizeof(TerrainVertex);

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = pVertices;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    hres = pDevice->CreateBuffer(&vbDesc, &initData, &m_pMeshVB);
    if (FAILED(hres))
    {
        return false;
    }
    delete(pVertices);

    // Index buffer
    m_numIndices = ((m_meshDim + 1) * 2) * (m_meshDim)+(m_meshDim - 1);

    int idDim = m_meshDim + 1;

    int *pIndices = new int[m_numIndices];

    int index = 0;
    for (int z = 0; z < idDim - 1; z++)
    {
        if (z % 2 == 0)
        {
            // Even row
            int x;
            for (x = 0; x < idDim; x++)
            {
                pIndices[index++] = x + (z * idDim);
                pIndices[index++] = x + (z * idDim) + idDim;
            }
            // degenerate vertex if not the last row
            if (z != idDim - 2)
            {
                pIndices[index++] = --x + (z * idDim);
            }
        }
        else
        {
            // Odd row
            int x;
            for (x = idDim - 1; x >= 0; x--)
            {
                pIndices[index++] = x + (z * idDim);
                pIndices[index++] = x + (z * idDim) + idDim;
            }
            // degenerate vertex if not the last row
            if (z != idDim - 2)
            {
                pIndices[index++] = ++x + (z * idDim);
            }
        }
    }

    D3D11_BUFFER_DESC ibDesc;
    ibDesc.ByteWidth = m_numIndices * sizeof(int);
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;
    ibDesc.MiscFlags = 0;
    ibDesc.StructureByteStride = sizeof(int);

    initData.pSysMem = pIndices;

    hres = pDevice->CreateBuffer(&ibDesc, &initData, &m_pMeshIB);
    if (FAILED(hres))
    {
        return false;
    }

    delete(pIndices);

    return true;
}