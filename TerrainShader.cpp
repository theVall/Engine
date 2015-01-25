#include "TerrainShader.h"


TerrainShader::TerrainShader()
{
    m_pLightBuffer = 0;
    m_pCameraBuffer = 0;
}


TerrainShader::TerrainShader(const TerrainShader &)
{
}


TerrainShader::~TerrainShader()
{
}


bool TerrainShader::InitializeShader(ID3D11Device *device,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *psFilename)
{
    HRESULT result;
    ID3D10Blob *errorMessage;
    ID3D10Blob *vertexShaderBuffer;
    ID3D10Blob *pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[4];
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC lightBufferDesc;

    unsigned int numElements = 0;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // Compile the vertex shader code.
    result = D3DCompileFromFile(vsFilename,
                                NULL,
                                NULL,
                                "Main",
                                "vs_5_0",
                                NULL,
                                NULL,
                                &vertexShaderBuffer,
                                &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile show the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // If there was nothing in the error message then the file could not be found.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing shader file", MB_OK);
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
                                &pixelShaderBuffer,
                                &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile show error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // If there was nothing in the error message then the file could not be found.
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                        vertexShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_pVertexShader);
    if (FAILED(result))
    {
        return false;
    }

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
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

    result = device->CreateInputLayout(polygonLayout,
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

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
    result = device->CreateSamplerState(&samplerDesc, &m_pSamplerState);
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
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
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
    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void TerrainShader::ShutdownShader()
{
    SafeRelease(m_pLightBuffer);
    SafeRelease(m_pCameraBuffer);

    return;
}


bool TerrainShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                        const XMMATRIX &worldMatrix,
                                        const XMMATRIX &viewMatrix,
                                        const XMMATRIX &projectionMatrix,
                                        const XMFLOAT3 &lightDirection,
                                        const XMFLOAT4 &ambientColor,
                                        const XMFLOAT4 &diffuseColor,
                                        const vector<Texture *> vTextures,
                                        const float scaling)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *transformDataBuffer;
    LightBufferType *lightDataBuffer;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = pContext->Map(m_pMatrixBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    transformDataBuffer = (MatrixBufferType *)mappedResource.pData;

    // Transpose the matrices for shader and copy them into the constant buffer.
    transformDataBuffer->world = XMMatrixTranspose(worldMatrix);
    transformDataBuffer->view = XMMatrixTranspose(viewMatrix);
    transformDataBuffer->projection = XMMatrixTranspose(projectionMatrix);

    // Unlock the constant buffer.
    pContext->Unmap(m_pMatrixBuffer, 0);
    bufferNumber = 0;
    pContext->VSSetConstantBuffers(bufferNumber, 1, &m_pMatrixBuffer);

    // set texture resources for pixel shader
    ID3D11ShaderResourceView *pSrvs[5];

    for (size_t i = 0; i < vTextures.size(); ++i)
    {
        pSrvs[i] = vTextures.at(i)->GetSrv();
    }

    pContext->PSSetShaderResources(0, vTextures.size(), pSrvs);

    // Lock the light constant buffer so it can be written to.
    result = pContext->Map(m_pLightBuffer,
                           0,
                           D3D11_MAP_WRITE_DISCARD,
                           0,
                           &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    lightDataBuffer = (LightBufferType *)mappedResource.pData;

    lightDataBuffer->ambientColor = ambientColor;
    lightDataBuffer->diffuseColor = diffuseColor;
    lightDataBuffer->lightDirection = lightDirection;
    lightDataBuffer->scaling = scaling;

    // Unlock the constant buffer.
    pContext->Unmap(m_pLightBuffer, 0);
    bufferNumber = 0;
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
    pContext->PSSetShader(m_pPixelShader, NULL, 0);
    pContext->PSSetSamplers(0, 1, &m_pSamplerState);

    pContext->DrawIndexed(indexCount, 0, 0);

    // Do NOT Unbind SRV here - multiple render calls (-> quad tree)
    //ID3D11ShaderResourceView *pNullSrv[1] = { NULL };
    //pContext->PSSetShaderResources(0, 1, pNullSrv);

    return;
}