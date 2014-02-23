#pragma once

#include "ShaderProgram.h"

class TerrainShader :
    public ShaderProgram
{
private:
    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT3 lightDirection;
        float padding;
    };

public:
    TerrainShader();
    TerrainShader(const TerrainShader &);
    ~TerrainShader();

    bool SetShaderParameters(ID3D11DeviceContext *deviceContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             ID3D11ShaderResourceView* texture,
                             const XMFLOAT3 &lightDirection,
                             const XMFLOAT4 &ambientColor,
                             const XMFLOAT4 &diffuseColor);

    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
    void ShutdownShader();

private:
    ID3D11Buffer* m_cameraBuffer;
    ID3D11Buffer* m_lightBuffer;
};

