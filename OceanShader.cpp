#include "OceanShader.h"


OceanShader::OceanShader(int meshDim)
{
    m_meshDim = meshDim;
}


OceanShader::OceanShader(const OceanShader &)
{
}


OceanShader::~OceanShader()
{
}


bool OceanShader::InitializeShader(ID3D11Device *pDevice,
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
    ID3D10Blob *pBufferWireframePS = NULL;

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
                                "OceanPS",
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

    result = D3DCompileFromFile(psFilename,
                                NULL,
                                NULL,
                                "WireframePS",
                                "ps_5_0",
                                NULL,
                                NULL,
                                &pBufferWireframePS,
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
                                         &m_pOceanSurfaceVS);
    if (FAILED(result))
    {
        return false;
    }

    // Create the pixel shaders.
    result = pDevice->CreatePixelShader(pBufferPS->GetBufferPointer(),
                                        pBufferPS->GetBufferSize(),
                                        NULL,
                                        &m_pOceanSurfacePS);
    if (FAILED(result))
    {
        return false;
    }
    result = pDevice->CreatePixelShader(pBufferWireframePS->GetBufferPointer(),
                                        pBufferWireframePS->GetBufferSize(),
                                        NULL,
                                        &m_pWireframePS);
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
                                        &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // clean up shader buffers
    pBufferVS->Release();
    pBufferVS = 0;

    pBufferPS->Release();
    pBufferPS = 0;

    pBufferWireframePS->Release();
    pBufferWireframePS = 0;

    // Constant buffer
    // Constants
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    D3D11_BUFFER_DESC perFameBufferDesc;
    perFameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    perFameBufferDesc.ByteWidth = ((sizeof(PerFrameBufferType) + 15) / 16 * 16);
    perFameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    perFameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    perFameBufferDesc.MiscFlags = 0;
    perFameBufferDesc.StructureByteStride = 0;

    result = pDevice->CreateBuffer(&perFameBufferDesc, NULL, &m_perFameBuffer);
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

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.MaxAnisotropy = 8;
    result = pDevice->CreateSamplerState(&samplerDesc, &m_pGradientSampler);
    if (FAILED(result))
    {
        return false;
    }

    result = pDevice->CreateSamplerState(&samplerDesc, &m_pSkyDomeSampler);
    if (FAILED(result))
    {
        return false;
    }

    // State blocks
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = 0;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.MultisampleEnable = TRUE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    result = pDevice->CreateRasterizerState(&rasterDesc, &m_pRsStateSolid);
    if (FAILED(result))
    {
        return false;
    }

    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    result = pDevice->CreateRasterizerState(&rasterDesc, &m_pRsStateWireframe);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void OceanShader::ShutdownShader()
{
    // D3D Buffer
    if (m_pMeshIB)
    {
        m_pMeshIB->Release();
        m_pMeshIB = 0;
    }
    if (m_pMeshVB)
    {
        m_pMeshVB->Release();
        m_pMeshVB = 0;
    }
    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }
    if (m_perFameBuffer)
    {
        m_perFameBuffer->Release();
        m_perFameBuffer = 0;
    }
    // Shader
    if (m_pOceanSurfaceVS)
    {
        m_pOceanSurfaceVS->Release();
        m_pOceanSurfaceVS = 0;
    }
    if (m_pOceanSurfacePS)
    {
        m_pOceanSurfacePS->Release();
        m_pOceanSurfacePS = 0;
    }
    if (m_pWireframePS)
    {
        m_pWireframePS->Release();
        m_pWireframePS = 0;
    }
    // Sampler
    if (m_pHeightSampler)
    {
        m_pHeightSampler->Release();
        m_pHeightSampler = 0;
    }
    if (m_pGradientSampler)
    {
        m_pGradientSampler->Release();
        m_pGradientSampler = 0;
    }
    if (m_pSkyDomeSampler)
    {
        m_pSkyDomeSampler->Release();
        m_pSkyDomeSampler = 0;
    }
    // State blocks
    if (m_pRsStateSolid)
    {
        m_pRsStateSolid->Release();
        m_pRsStateSolid = 0;
    }
    if (m_pRsStateWireframe)
    {
        m_pRsStateWireframe->Release();
        m_pRsStateWireframe = 0;
    }
    // Layout
    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }
}


bool OceanShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                      const XMMATRIX &worldMatrix,
                                      const XMMATRIX &viewMatrix,
                                      const XMMATRIX &projectionMatrix,
                                      const XMFLOAT3 &eyeVec,
                                      const XMFLOAT3 &lightDir,
                                      ID3D11ShaderResourceView *displacementTex,
                                      ID3D11ShaderResourceView *gradientTex,
                                      ID3D11ShaderResourceView *skyDomeTex)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *pTransformDataBuffer;

    // Lock the constant buffer so it can be written to.
    result = pContext->Map(m_matrixBuffer,
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

    pContext->Unmap(m_matrixBuffer, 0);
    pContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

    result = pContext->Map(m_perFameBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Other per frame constants
    PerFrameBufferType *pPerFrameDataBuffer = (PerFrameBufferType *)mappedResource.pData;
    pPerFrameDataBuffer->eyeVec = eyeVec;
    pPerFrameDataBuffer->lightDir = lightDir;
    pContext->Unmap(m_perFameBuffer, 0);
    pContext->PSSetConstantBuffers(1, 1, &m_perFameBuffer);

    // Set textures.
    ID3D11ShaderResourceView *pSrvs[3] = { displacementTex, gradientTex, skyDomeTex };

    pContext->VSSetShaderResources(0, 3, &pSrvs[0]);
    pContext->PSSetShaderResources(0, 3, &pSrvs[0]);

    return true;
}


bool OceanShader::Render(ID3D11DeviceContext *pContext,
                         const XMMATRIX &worldMatrix,
                         const XMMATRIX &viewMatrix,
                         const XMMATRIX &projectionMatrix,
                         const XMFLOAT3 &eyeVec,
                         const XMFLOAT3 &lightDir,
                         ID3D11ShaderResourceView *displacementTex,
                         ID3D11ShaderResourceView *gradientTex,
                         ID3D11ShaderResourceView *skyDomeTex)
{
    if (!SetShaderParameters(pContext,
                             worldMatrix,
                             viewMatrix,
                             projectionMatrix,
                             eyeVec,
                             lightDir,
                             displacementTex,
                             gradientTex,
                             skyDomeTex))
    {
        return false;
    }

    // Set the vertex and index buffers to active in the input assembler.
    unsigned int stride = sizeof(OceanVertex);
    unsigned int offset = 0;

    pContext->IASetVertexBuffers(0, 1, &m_pMeshVB, &stride, &offset);
    pContext->IASetIndexBuffer(m_pMeshIB, DXGI_FORMAT_R32_UINT, 0);

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Render
    RenderShader(pContext);

    return true;
}


void OceanShader::RenderShader(ID3D11DeviceContext *pContext)
{
    //pContext->RSSetState(m_pRsStateWireframe);

    // Set the vertex input layout.
    pContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders.
    pContext->VSSetShader(m_pOceanSurfaceVS, NULL, 0);
    pContext->PSSetShader(m_pOceanSurfacePS, NULL, 0);
    //pContext->PSSetShader(m_pWireframePS, NULL, 0);

    // Sampler
    ID3D11SamplerState *vs_samplers[3] = { m_pHeightSampler,
                                           m_pGradientSampler,
                                           m_pSkyDomeSampler
                                         };
    pContext->VSSetSamplers(0, 3, &vs_samplers[0]);
    ID3D11SamplerState *ps_samplers[3] = { m_pHeightSampler,
                                           m_pGradientSampler,
                                           m_pSkyDomeSampler
                                         };
    pContext->PSSetSamplers(1, 3, &ps_samplers[0]);

    // draw call
    pContext->DrawIndexed(m_numIndices, 0, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView *pNullSrvs[2] = { NULL, NULL };
    pContext->VSSetShaderResources(0, 2, pNullSrvs);
    pContext->PSSetShaderResources(0, 2, pNullSrvs);

    return;
}


bool OceanShader::CreateSurfaceVertices(ID3D11Device *pDevice)
{
    HRESULT hres;

    // Vertex buffer
    int numVertices = (m_meshDim + 1) * (m_meshDim + 1);
    OceanVertex *pVertices = new OceanVertex[numVertices];

    for (int i = 0; i <= m_meshDim; i++)
    {
        for (int j = 0; j <= m_meshDim; j++)
        {
            pVertices[i * (m_meshDim + 1) + j].indexX = (float)j;
            pVertices[i * (m_meshDim + 1) + j].indexY = (float)i;
        }
    }

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth = numVertices * sizeof(OceanVertex);
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = sizeof(OceanVertex);

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
    m_numIndices = ((m_meshDim + 1) * 2) * (m_meshDim) + (m_meshDim - 1);

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