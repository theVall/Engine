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

    bool Render(ID3D11DeviceContext* deviceContext,
                int indexCount,
                const XMMATRIX &worldMatrix,
                const XMMATRIX &viewMatrix,
                const XMMATRIX &projectionMatrix,
                const XMFLOAT4 &apexColor,
                const XMFLOAT4 &centerColor);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             const XMFLOAT4 &apexColor,
                             const XMFLOAT4 &centerColor);

    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11Buffer* m_gradientBuffer;
};

