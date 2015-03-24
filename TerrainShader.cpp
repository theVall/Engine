#include "TerrainShader.h"


TerrainShader::TerrainShader()
{
    m_pLightBuffer = 0;
    m_pPerFrameBuffer = 0;
}


TerrainShader::TerrainShader(const TerrainShader &)
{
}


TerrainShader::~TerrainShader()
{
}


bool TerrainShader::InitializeShader(ID3D11Device *pDevice,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *hsFilename,
                                     WCHAR *dsFilename,
                                     WCHAR *psFilename)
{
    ID3D10Blob *vertexShaderBuffer = 0;
    ID3D10Blob *hullShaderBuffer = 0;
    ID3D10Blob *domainShaderBuffer = 0;
    ID3D10Blob *pixelShaderBuffer = 0;

    D3D11_INPUT_ELEMENT_DESC polygonLayout[4];
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC lightBufferDesc;
    D3D11_BUFFER_DESC perFrameBufferDesc;

    unsigned int numElements = 0;

    // Compile the vertex shader code.
    if (!CompileShader(vsFilename,
                       "Main",
                       "vs_5_0",
                       &vertexShaderBuffer,
                       hwnd))
    {
        return false;
    }

    // Compile the hull shader code.
    if (!CompileShader(hsFilename,
                       "Main",
                       "hs_5_0",
                       &hullShaderBuffer,
                       hwnd))
    {
        return false;
    }
    // Compile the domain shader code.
    if (!CompileShader(dsFilename,
                       "Main",
                       "ds_5_0",
                       &domainShaderBuffer,
                       hwnd))
    {
        return false;
    }
    // Compile the pixel shader code.
    if (!CompileShader(psFilename,
                       "Main",
                       "ps_5_0",
                       &pixelShaderBuffer,
                       hwnd))
    {
        return false;
    }

    HRESULT result = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                                 vertexShaderBuffer->GetBufferSize(),
                                                 NULL,
                                                 &m_pVertexShader);
    if (FAILED(result))
    {
        return false;
    }

    result = pDevice->CreateHullShader(hullShaderBuffer->GetBufferPointer(),
                                       hullShaderBuffer->GetBufferSize(),
                                       NULL,
                                       &m_pHullShader);
    if (FAILED(result))
    {
        return false;
    }
    result = pDevice->CreateDomainShader(domainShaderBuffer->GetBufferPointer(),
                                         domainShaderBuffer->GetBufferSize(),
                                         NULL,
                                         &m_pDomainShader);
    if (FAILED(result))
    {
        return false;
    }

    result = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                        pixelShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pPixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // Create the vertex input layout description.
    // This setup needs to match the VertexType structure in the Terrain class and shader.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    polygonLayout[3].SemanticName = "COLOR";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    result = pDevice->CreateInputLayout(polygonLayout,
                                        numElements,
                                        vertexShaderBuffer->GetBufferPointer(),
                                        vertexShaderBuffer->GetBufferSize(),
                                        &m_pLayout);
    if (FAILED(result))
    {
        return false;
    }

    // Release the vertex shader buffer and pixel shader buffer
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    hullShaderBuffer->Release();
    hullShaderBuffer = 0;

    domainShaderBuffer->Release();
    domainShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    // Create the texture sampler state.
    result = pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);
    if (FAILED(result))
    {
        return false;
    }

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Setup the description of the light dynamic constant buffer that is in the pixel shader.
    // ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER.
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;
    result = pDevice->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    perFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    perFrameBufferDesc.ByteWidth = sizeof(PerFrameBufferType);
    perFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    perFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    perFrameBufferDesc.MiscFlags = 0;
    perFrameBufferDesc.StructureByteStride = 0;
    result = pDevice->CreateBuffer(&perFrameBufferDesc, NULL, &m_pPerFrameBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void TerrainShader::ShutdownShader()
{
    SafeRelease(m_pLightBuffer);
    SafeRelease(m_pPerFrameBuffer);

    return;
}


bool TerrainShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                        const XMMATRIX &worldMatrix,
                                        const XMMATRIX &viewMatrix,
                                        const XMMATRIX &projectionMatrix,
                                        const XMFLOAT3 &lightDirection,
                                        const XMFLOAT4 &ambientColor,
                                        const XMFLOAT4 &diffuseColor,
                                        const XMFLOAT3 &eyePosition,
                                        const vector<Texture *> vTextures,
                                        Texture *pNoiseTex,
                                        const float scaling)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *pTransformDataBuffer;
    LightBufferType *pLightDataBuffer;
    PerFrameBufferType *pPerFrameBuffer;
    unsigned int bufferNumber;

    // Update the matrices in the constant buffer.
    result = pContext->Map(m_pMatrixBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }
    pTransformDataBuffer = (MatrixBufferType *)mappedResource.pData;
    // Transpose the matrices for shader and copy them into the constant buffer.
    pTransformDataBuffer->world = XMMatrixTranspose(worldMatrix);
    pTransformDataBuffer->view = XMMatrixTranspose(viewMatrix);
    pTransformDataBuffer->projection = XMMatrixTranspose(projectionMatrix);
    pContext->Unmap(m_pMatrixBuffer, 0);

    // Update the values in the misc constant buffer.
    result = pContext->Map(m_pPerFrameBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }
    // Get a pointer to the data in the constant buffer.
    pPerFrameBuffer = (PerFrameBufferType *)mappedResource.pData;
    // update values
    pPerFrameBuffer->eyeVec = eyePosition;
    pPerFrameBuffer->tessFact = XMFLOAT3(7.0f, 7.0f, 3.5f);
    pContext->Unmap(m_pPerFrameBuffer, 0);

    // Create array containing matrix and misc constant buffers.
    ID3D11Buffer *constBuffers[2];
    constBuffers[0] = m_pMatrixBuffer;
    constBuffers[1] = m_pPerFrameBuffer;

    // Light parameter constant buffer.
    result = pContext->Map(m_pLightBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }
    pLightDataBuffer = (LightBufferType *)mappedResource.pData;
    pLightDataBuffer->ambientColor = ambientColor;
    pLightDataBuffer->diffuseColor = diffuseColor;
    pLightDataBuffer->lightDirection = lightDirection;
    pLightDataBuffer->scaling = scaling;
    pContext->Unmap(m_pLightBuffer, 0);

    bufferNumber = 0;
    // Vertex shader buffers
    pContext->VSSetConstantBuffers(bufferNumber, 2, &constBuffers[0]);
    // Hull shader buffers
    pContext->HSSetConstantBuffers(bufferNumber, 1, &constBuffers[1]);
    // Domain shader buffers
    pContext->DSSetConstantBuffers(bufferNumber, 2, &constBuffers[0]);
    ID3D11ShaderResourceView *pSrv = pNoiseTex->GetSrv();
    pContext->DSSetShaderResources(0, 1, &pSrv);
    // Pixel shader buffers
    // set texture resources for pixel shader
    ID3D11ShaderResourceView *pSrvs[5];
    for (size_t i = 0; i < vTextures.size(); ++i)
    {
        pSrvs[i] = vTextures.at(i)->GetSrv();
    }

    pContext->PSSetShaderResources(1, vTextures.size(), pSrvs);
    pContext->PSSetConstantBuffers(bufferNumber, 1, &m_pLightBuffer);

    return true;
}


void TerrainShader::RenderShader(ID3D11DeviceContext *pContext,
                                 int indexCount,
                                 bool wireframe)
{
    pContext->IASetInputLayout(m_pLayout);

    if (wireframe)
    {
        pContext->RSSetState(m_pRsStateWireframe);
    }

    // Set the vertex and pixel shader.
    pContext->VSSetShader(m_pVertexShader, NULL, 0);
    pContext->HSSetShader(m_pHullShader, NULL, 0);
    pContext->DSSetShader(m_pDomainShader, NULL, 0);
    pContext->DSSetSamplers(0, 1, &m_pSamplerState);
    pContext->PSSetShader(m_pPixelShader, NULL, 0);
    pContext->PSSetSamplers(0, 1, &m_pSamplerState);

    pContext->DrawIndexed(indexCount, 0, 0);

    // !! Do NOT Unbind SRV here - multiple render calls (-> quad tree) !!
    pContext->DSSetShaderResources(0, NULL, NULL);

    // Unbind tessellation shaders to clear them from context for next render passes
    // without tessellation.
    pContext->HSSetShader(NULL, NULL, 0);
    pContext->DSSetShader(NULL, NULL, 0);

    return;
}