#include "ShaderProgram.h"


ShaderProgram::ShaderProgram()
{
    m_pVertexShader = 0;
    m_pPixelShader = 0;
    m_pLayout = 0;
    m_pSamplerState = 0;

    m_pMatrixBuffer = 0;
}


ShaderProgram::ShaderProgram(const ShaderProgram &)
{
}


ShaderProgram::~ShaderProgram()
{
}


bool ShaderProgram::SetRasterStates(ID3D11Device *pDevice)
{
    HRESULT hres;

    // Create wireframe state
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = 0;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.MultisampleEnable = TRUE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    hres = pDevice->CreateRasterizerState(&rasterDesc, &m_pRsStateWireframe);
    if (FAILED(hres))
    {
        return false;
    }

    // Create solid state
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    hres = pDevice->CreateRasterizerState(&rasterDesc, &m_pRsStateSolid);
    if (FAILED(hres))
    {
        return false;
    }

    return true;
}


bool ShaderProgram::Initialize(ID3D11Device *pDevice,
                               HWND hwnd,
                               WCHAR *pVsFilename,
                               WCHAR *pPsFilename)
{
    SetRasterStates(pDevice);

    // Call object specific initialization method.
    if (!InitializeShader(pDevice,
                          hwnd,
                          pVsFilename,
                          pPsFilename))
    {
        return false;
    }

    return true;
}


bool ShaderProgram::Initialize(ID3D11Device *pDevice,
                               HWND hwnd,
                               WCHAR *pVsFilename,
                               WCHAR *pHsFilename,
                               WCHAR *pDsFilename,
                               WCHAR *pPsFilename)
{
    SetRasterStates(pDevice);

    if (!InitializeShader(pDevice,
                          hwnd,
                          pVsFilename,
                          pHsFilename,
                          pDsFilename,
                          pPsFilename))
    {
        return false;
    }

    return true;
}


void ShaderProgram::Shutdown()
{
    // Release general shader program objects.
    SafeRelease(m_pPixelShader);
    SafeRelease(m_pHullShader);
    SafeRelease(m_pDomainShader);
    SafeRelease(m_pVertexShader);

    SafeRelease(m_pSamplerState);
    SafeRelease(m_pLayout);

    SafeRelease(m_pMatrixBuffer);

    SafeRelease(m_pRsStateWireframe);

    // Shutdown object specific objects.
    ShutdownShader();

    return;
}


bool ShaderProgram::CompileShader(WCHAR *shaderFilename,
                                  LPCSTR pEntryPoint,
                                  LPCSTR pTarget,
                                  ID3DBlob **pCode,
                                  HWND hwnd)
{
    ID3D10Blob *errorMessage;

    HRESULT result = D3DCompileFromFile(shaderFilename,
                                        NULL,
                                        NULL,
                                        pEntryPoint,
                                        pTarget,
                                        NULL,
                                        NULL,
                                        pCode,
                                        &errorMessage);

    if (FAILED(result))
    {
        // If the shader failed to compile show the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, shaderFilename);
        }
        // If there was nothing in the error message then the file could not be found.
        else
        {
            MessageBox(hwnd, shaderFilename, L"Missing shader file", MB_OK);
        }

        return false;
    }

    return true;
}


void ShaderProgram::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename)
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


bool ShaderProgram::InitializeShader(ID3D11Device *pDevice,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *psFilename)
{
    // Workaround dummy method for linking error with the shader classes
    // using the tessellation stage.

    return true;
}


bool ShaderProgram::InitializeShader(ID3D11Device *pDevice,
                                     HWND hwnd,
                                     WCHAR *vsFilename,
                                     WCHAR *hsFilename,
                                     WCHAR *dsFilename,
                                     WCHAR *psFilename)
{
    // Workaround dummy method for linking error with the shader classes
    // not actually using the tessellation stage.

    return true;
}
