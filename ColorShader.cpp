#include "ColorShader.h"


ColorShader::ColorShader(void)
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
}


ColorShader::ColorShader(const ColorShader &)
{
}


ColorShader::~ColorShader(void)
{
}


bool ColorShader::Initialize(ID3D11Device *device, HWND hwnd)
{
    bool result;

    result = InitializeShader(device,
                              hwnd,
                              L"./shader/VertexShader.hlsl",
                              L"./shader/PixelShader.hlsl");
    if (!result)
    {
        return false;
    }

    return true;
}


void ColorShader::Shutdown()
{
    // Shutdown the vertex and pixel shader as well as the related objects.
    ShutdownShader();

    return;
}


bool ColorShader::Render(ID3D11DeviceContext *deviceContext,
                         int indexCount,
                         const XMMATRIX &worldMatrix,
                         const XMMATRIX &viewMatrix,
                         const XMMATRIX &projectionMatrix)
{
    bool result;

    // Set the shader parameters for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
    if (!result)
    {
        return false;
    }

    // Render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool ColorShader::InitializeShader(ID3D11Device *device,
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

    // Initialize the pointers this function will use to null.
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
        //  If the shader failed to compile show error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        //  If there was nothing in the error message then the file could not be found.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    //  Compile the pixel shader code.
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
        //  If the shader failed to compile show error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        //  If there was nothing in the error message then the file could not be found.
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                        vertexShaderBuffer->GetBufferSize(),
                                        NULL,
                                        &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                       pixelShaderBuffer->GetBufferSize(),
                                       NULL,
                                       &m_pixelShader);
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

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

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

    //  Create the constant buffer pointer so we can access the vertex shader constant buffer
    //  from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


bool ColorShader::SetShaderParameters(ID3D11DeviceContext *deviceContext,
                                      const XMMATRIX &worldMatrix,
                                      const XMMATRIX &viewMatrix,
                                      const XMMATRIX &projectionMatrix)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType *dataPtr;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (MatrixBufferType *)mappedResource.pData;

    // Transpose the matrices for shader and copy them into the constant buffer.
    dataPtr->world      = XMMatrixTranspose(worldMatrix);
    dataPtr->view       = XMMatrixTranspose(viewMatrix);
    dataPtr->projection = XMMatrixTranspose(projectionMatrix);

    deviceContext->Unmap(m_matrixBuffer, 0);
    bufferNumber = 0;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    return true;
}


void ColorShader::RenderShader(ID3D11DeviceContext *deviceContext, int indexCount)
{
    deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shader.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}


void ColorShader::ShutdownShader()
{
    if(m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    if(m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }

    if(m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    if(m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }

    return;
}


void ColorShader::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                           HWND hwnd,
                                           WCHAR *shaderFilename)
{
    char *compileErrors;
    unsigned long bufferSize, i;
    std::ofstream fout;

    compileErrors = (char *)(errorMessage->GetBufferPointer());

    bufferSize = errorMessage->GetBufferSize();

    fout.open("ShaderErrorLog.txt");

    for(i=0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"Error compiling shader. Check ShaderErrorLog.txt for details.", shaderFilename, MB_OK);

    return;
}
