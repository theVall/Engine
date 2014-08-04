#include "SkyDomeShader.h"


SkyDomeShader::SkyDomeShader()
{
    m_gradientBuffer = 0;
}


SkyDomeShader::SkyDomeShader(const SkyDomeShader &)
{
}


SkyDomeShader::~SkyDomeShader()
{
}


bool SkyDomeShader::Render(ID3D11DeviceContext *deviceContext,
                           int indexCount,
                           const XMMATRIX &worldMatrix,
                           const XMMATRIX &viewMatrix,
                           const XMMATRIX &projectionMatrix,
                           const XMFLOAT4 &apexColor,
                           const XMFLOAT4 &centerColor,
                           ID3D11ShaderResourceView *skyDomeTex)
{
    if (!SetShaderParameters(deviceContext,
                             worldMatrix,
                             viewMatrix,
                             projectionMatrix,
                             apexColor,
                             centerColor,
                             skyDomeTex))
    {
        return false;
    }

    RenderShader(deviceContext, indexCount);

    return true;
}


// TODO -> move (parts) to base class
bool SkyDomeShader::InitializeShader(ID3D11Device *device,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *psFilename)
{
    HRESULT result;

    ID3D10Blob *errorMessage = 0;
    ID3D10Blob *vertexShaderBuffer = 0;
    ID3D10Blob *pixelShaderBuffer = 0;

    unsigned int numElements;

    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC gradientBufferDesc;

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
        // If the shader failed to compile it should have written something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
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
                                &pixelShaderBuffer,
                                &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile,
        // it should have written something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // If there is nothing in the error message then the file itself could not be found.
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                        vertexShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                       pixelShaderBuffer->GetBufferSize(),
                                       NULL,
                                       &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // Create the vertex input layout description.
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

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

    // Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout,
                                       numElements,
                                       vertexShaderBuffer->GetBufferPointer(),
                                       vertexShaderBuffer->GetBufferSize(),
                                       &m_layout);
    if (FAILED(result))
    {
        return false;
    }

// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

// Setup the description of the gradient constant buffer that is in the pixel shader.
    gradientBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    gradientBufferDesc.ByteWidth = sizeof(GradientBufferType);
    gradientBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    gradientBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    gradientBufferDesc.MiscFlags = 0;
    gradientBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
    result = device->CreateBuffer(&gradientBufferDesc, NULL, &m_gradientBuffer);
    if (FAILED(result))
    {
        return false;
    }

// Samplers
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, &m_pSkySampler);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void SkyDomeShader::ShutdownShader()
{
    if (m_pSkySampler)
    {
        m_pSkySampler->Release();
        m_pSkySampler = 0;
    }

    if (m_gradientBuffer)
    {
        m_gradientBuffer->Release();
        m_gradientBuffer = 0;
    }

    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }

    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }

    return;
}


bool SkyDomeShader::SetShaderParameters(ID3D11DeviceContext *deviceContext,
                                        const XMMATRIX &worldMatrix,
                                        const XMMATRIX &viewMatrix,
                                        const XMMATRIX &projectionMatrix,
                                        const XMFLOAT4 &apexColor,
                                        const XMFLOAT4 &centerColor,
                                        ID3D11ShaderResourceView *skyDomeTex)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *pTransformDataBuffer;
    GradientBufferType *pGradientDataBuffer;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer,
                                0,
                                D3D11_MAP_WRITE_DISCARD,
                                0,
                                &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    pTransformDataBuffer = (MatrixBufferType *)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    pTransformDataBuffer->world = XMMatrixTranspose(worldMatrix);
    pTransformDataBuffer->view = XMMatrixTranspose(viewMatrix);
    pTransformDataBuffer->projection = XMMatrixTranspose(projectionMatrix);

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);
    bufferNumber = 0;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


    // Lock the gradient constant buffer so it can be written to.
    result = deviceContext->Map(m_gradientBuffer,
                                0,
                                D3D11_MAP_WRITE_DISCARD,
                                0,
                                &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    pGradientDataBuffer = (GradientBufferType *)mappedResource.pData;

    // Copy the gradient color variables into the constant buffer.
    pGradientDataBuffer->apexColor = apexColor;
    pGradientDataBuffer->centerColor = centerColor;

    deviceContext->PSSetShaderResources(0, 1, &skyDomeTex);
    // Unlock the constant buffer.
    deviceContext->Unmap(m_gradientBuffer, 0);
    bufferNumber = 0;
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_gradientBuffer);

    return true;
}


void SkyDomeShader::RenderShader(ID3D11DeviceContext *deviceContext, int indexCount)
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    deviceContext->PSSetSamplers(0, 1, &m_pSkySampler);

    deviceContext->DrawIndexed(indexCount, 0, 0);

    // Unbind SRV
    ID3D11ShaderResourceView *pNullSrv[1] = { NULL };
    deviceContext->PSSetShaderResources(0, 1, pNullSrv);

    return;
}
