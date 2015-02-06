#include "LineShader.h"


LineShader::LineShader()
{
}


LineShader::LineShader(const LineShader &)
{
}


LineShader::~LineShader()
{
}


bool LineShader::InitializeShader(ID3D11Device *pDevice,
                                  HWND hwnd,
                                  WCHAR *vsFilename,
                                  WCHAR *psFilename)
{
    HRESULT result;
    ID3D10Blob *errorMessage;
    ID3D10Blob *vertexShaderBuffer;
    ID3D10Blob *pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;

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

    result = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                         vertexShaderBuffer->GetBufferSize(),
                                         NULL,
                                         &m_pVertexShader);
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

    //  Setup the layout of shader data.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

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

    result = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void LineShader::ShutdownShader()
{
}


bool LineShader::SetShaderParameters(ID3D11DeviceContext *pContext,
                                     const XMMATRIX &worldMatrix,
                                     const XMMATRIX &viewMatrix,
                                     const XMMATRIX &projectionMatrix)
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

    return true;
}


void LineShader::RenderShader(ID3D11DeviceContext *pContext, int indexCount)
{
    pContext->IASetInputLayout(m_pLayout);

    // Set the vertex and pixel shader.
    pContext->VSSetShader(m_pVertexShader, NULL, 0);
    pContext->PSSetShader(m_pPixelShader, NULL, 0);

    pContext->DrawIndexed(indexCount, 0, 0);

    return;
}
