#pragma once

#include "ShaderProgram.h"

class SkyDomeShader :
    public ShaderProgram
{
private:
    // PS shader type
    struct GradientBufferType
    {
        XMFLOAT4 apexColor;
        XMFLOAT4 centerColor;
    };

public:
    SkyDomeShader();
    SkyDomeShader(const SkyDomeShader &);
    ~SkyDomeShader();

    bool Render(ID3D11DeviceContext *deviceContext,
                int indexCount,
                const XMMATRIX &worldMatrix,
                const XMMATRIX &viewMatrix,
                const XMMATRIX &projectionMatrix,
                const XMFLOAT4 &apexColor,
                const XMFLOAT4 &centerColor,
                ID3D11ShaderResourceView *skyDomeTex);

private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext *deviceContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             const XMFLOAT4 &apexColor,
                             const XMFLOAT4 &centerColor,
                             ID3D11ShaderResourceView *skyDomeTex);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount);

private:
    ID3D11Buffer *m_gradientBuffer;
    ID3D11SamplerState *m_pSkySampler;
};

