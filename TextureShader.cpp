#include "TextureShader.h"


TextureShader::TextureShader(void)
{
}


TextureShader::TextureShader(const TextureShader &)
{
}


TextureShader::~TextureShader(void)
{
}


bool TextureShader::Render(ID3D11DeviceContext *pContext,
                           int indexCount,
                           const XMMATRIX &worldMatrix,
                           const XMMATRIX &viewMatrix,
                           const XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           float width,
                           float height,
                           float xRes,
                           float yRes,
                           Vec2f poi,
                           Vec2f poi2)
{
    bool result;

    // Set the shader parameters for rendering.
    result = SetShaderParameters(pContext,
                                 worldMatrix,
                                 viewMatrix,
                                 projectionMatrix,
                                 texture,
                                 width,
                                 height,
                                 xRes,
                                 yRes,
                                 poi,
                                 poi2);
    if (!result)
    {
        return false;
    }

    // Render the prepared buffers with the shader.
    RenderShader(pContext, indexCount);

    return true;
}


bool TextureShader::InitializeShader(ID3D11Device *device,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *psFilename)
{
    HRESULT result;
    ID3D10Blob *errorMessage;
    ID3D10Blob *vertexShaderBuffer;
    ID3D10Blob *pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

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

    //  Setup the layout of shader data.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

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

    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Description of the dynamic matrix constant buffer.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
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

    result = device->CreateBuffer(&perFameBufferDescPS, NULL, &m_perFameBufferPS);
    if (FAILED(result))
    {
        return false;
    }

    // Texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
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

    return true;
}


bool TextureShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                        const XMMATRIX &worldMatrix,
                                        const XMMATRIX &viewMatrix,
                                        const XMMATRIX &projectionMatrix,
                                        ID3D11ShaderResourceView *pSrv,
                                        float width,
                                        float height,
                                        float xRes,
                                        float yRes,
                                        Vec2f poi,
                                        Vec2f poi2)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *transformDataBuffer;
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
    pPerFrameDataBufferPS->width = width;
    pPerFrameDataBufferPS->height = height;
    pPerFrameDataBufferPS->xRes = xRes;
    pPerFrameDataBufferPS->yRes = yRes;
    pPerFrameDataBufferPS->poi = poi.GetAsXMFloat2();
    pPerFrameDataBufferPS->poi2 = poi2.GetAsXMFloat2();
    pContext->Unmap(m_perFameBufferPS, 0);
    pContext->PSSetConstantBuffers(1, 1, &m_perFameBufferPS);

    pContext->PSSetShaderResources(0, 1, &pSrv);

    return true;
}


void TextureShader::ShutdownShader()
{
}


void TextureShader::RenderShader(ID3D11DeviceContext *pContext, int indexCount)
{
    pContext->IASetInputLayout(m_pLayout);

    // Set the vertex and pixel shader.
    pContext->VSSetShader(m_pVertexShader, NULL, 0);
    pContext->PSSetShader(m_pPixelShader, NULL, 0);
    pContext->PSSetSamplers(0, 1, &m_pSamplerState);

    pContext->DrawIndexed(indexCount, 0, 0);

    // Unbind resources
    ID3D11ShaderResourceView *pNullSrv[1] = { NULL };
    pContext->PSSetShaderResources(0, 1, pNullSrv);

    return;
}