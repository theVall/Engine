#pragma once

#include "ShaderProgram.h"

class OceanShader :
    public ShaderProgram
{

public:
    OceanShader();
    OceanShader(const OceanShader &);
    ~OceanShader();

    bool SetShaderParameters(ID3D11DeviceContext *pContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             ID3D11ShaderResourceView *texture,
                             const XMFLOAT3 &lightDirection,
                             const XMFLOAT4 &ambientColor,
                             const XMFLOAT4 &diffuseColor);

    bool Render(ID3D11DeviceContext *pContext,
                const XMMATRIX &worldMatrix,
                const XMMATRIX &viewMatrix,
                const XMMATRIX &projectionMatrix);


private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext *peContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount);

private:

};

