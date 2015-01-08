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


bool ShaderProgram::Initialize(ID3D11Device *pDevice,
                               HWND hwnd,
                               WCHAR *pVsFilename,
                               WCHAR *pPsFilename)
{
    bool result;
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

    // Call object specific initialization method.
    result = InitializeShader(pDevice,
                              hwnd,
                              pVsFilename,
                              pPsFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


void ShaderProgram::Shutdown()
{
    // Release general shader program objects.
    SafeRelease(m_pPixelShader);
    SafeRelease(m_pVertexShader);

    SafeRelease(m_pSamplerState);
    SafeRelease(m_pLayout);

    SafeRelease(m_pMatrixBuffer);

    SafeRelease(m_pRsStateWireframe);

    // Shutdown object specific objects.
    ShutdownShader();

    return;
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
