#pragma once

#include "ShaderProgram.h"

class LineShader :
    public ShaderProgram
{
public:
    LineShader();
    LineShader(const LineShader &);
    ~LineShader();

    bool SetShaderParameters(ID3D11DeviceContext *pContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount);

private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();
};

