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
        float poiX;
        float poiY;
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
                float poiX,
                float poiY);

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
                             float poiX,
                             float poiY);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount);

    // member
private:

    ID3D11Buffer *m_perFameBufferPS;

};


