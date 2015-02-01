#pragma once

#include "ShaderProgram.h"

class TextureShader :
    public ShaderProgram
{
    struct PerFrameBufferTypePS
    {
        float width;
        float height;
        float xRes;
        float yRes;
        XMFLOAT2 poi;
        XMFLOAT2 poi2;
    };

public:
    TextureShader();
    TextureShader(const TextureShader &);
    ~TextureShader();

    bool Render(ID3D11DeviceContext *deviceContext,
                int indexCount,
                const XMMATRIX &worldMatrix,
                const XMMATRIX &viewMatrix,
                const XMMATRIX &projectionMatrix,
                ID3D11ShaderResourceView *texture,
                float width,
                float height,
                float xRes,
                float yRes,
                Vec2f poi,
                Vec2f poi2);

private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();
    bool SetShaderParameters(ID3D11DeviceContext *pContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             ID3D11ShaderResourceView *pSrv,
                             float width,
                             float height,
                             float xRes,
                             float yRes,
                             Vec2f poi,
                             Vec2f poi2);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount);

private:
    // member
    ID3D11Buffer *m_perFameBufferPS;

};


