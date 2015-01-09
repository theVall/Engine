#pragma once

#include "ShaderProgram.h"
#include <vector>
#include "Texture.h"

class TerrainShader :
    public ShaderProgram
{
private:
    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT3 lightDirection;
        float scaling;
    };

public:
    TerrainShader();
    TerrainShader(const TerrainShader &);
    ~TerrainShader();

    bool SetShaderParameters(ID3D11DeviceContext *pContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             const XMFLOAT3 &lightDirection,
                             const XMFLOAT4 &ambientColor,
                             const XMFLOAT4 &diffuseColor,
                             const vector<Texture *> vTextures,
                             const float scaling);

    void RenderShader(ID3D11DeviceContext *pContext, int indexCount, bool wireframe);

private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();

private:
    ID3D11Buffer *m_pCameraBuffer;
    ID3D11Buffer *m_pLightBuffer;
};

