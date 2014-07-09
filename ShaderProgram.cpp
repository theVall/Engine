#include "ShaderProgram.h"


ShaderProgram::ShaderProgram()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;

    m_matrixBuffer = 0;
}


ShaderProgram::ShaderProgram(const ShaderProgram &)
{
}


ShaderProgram::~ShaderProgram()
{
}


bool ShaderProgram::Initialize(ID3D11Device *device, HWND hwnd, WCHAR *vsFilename, WCHAR *psFilename)
{
    bool result;

    result = InitializeShader(device,
                              hwnd,
                              vsFilename,
                              psFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


void ShaderProgram::Shutdown()
{
    // Shutdown the vertex and pixel shaders as well as the related objects.
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
