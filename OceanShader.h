#pragma once

#include "ShaderProgram.h"

class OceanShader :
    public ShaderProgram
{

private:
    struct OceanVertex
    {
        float indexX;
        float indexY;
    };

    struct PerFrameBufferType
    {
        XMFLOAT3 eyeVec;
        XMFLOAT3 lightDir;
    };

public:
    OceanShader(int meshDim);
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
                const XMMATRIX &projectionMatrix,
                const XMFLOAT3 &eyeVec,
                const XMFLOAT3 &lightDir,
                ID3D11ShaderResourceView *displacementTex,
                ID3D11ShaderResourceView *gradientTex,
                ID3D11ShaderResourceView *skyDomeTex,
                bool wireframe);


private:
    bool InitializeShader(ID3D11Device *pDevice,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext *peContext,
                             const XMMATRIX &worldMatrix,
                             const XMMATRIX &viewMatrix,
                             const XMMATRIX &projectionMatrix,
                             const XMFLOAT3 &eyeVec,
                             const XMFLOAT3 &lightDir,
                             ID3D11ShaderResourceView *displacementTex,
                             ID3D11ShaderResourceView *gradientTex,
                             ID3D11ShaderResourceView *skyDomeTex
                            );

    void RenderShader(ID3D11DeviceContext *pContext, bool wireframe);

    // Create vertices for the ocean surface mesh
    bool CreateSurfaceVertices(ID3D11Device *pDevice);

private:
    int m_meshDim;
    int m_numIndices;
    int m_tileCount;

    ID3D11Buffer *m_pMeshVB;
    ID3D11Buffer *m_pMeshIB;

    // Shaders
    ID3D11VertexShader *m_pOceanSurfaceVS;
    ID3D11PixelShader *m_pOceanSurfacePS;
    ID3D11PixelShader *m_pWireframePS;

    // Buffers
    ID3D11Buffer *m_perFameBuffer;

    // Samplers
    ID3D11SamplerState *m_pHeightSampler;
    ID3D11SamplerState *m_pGradientSampler;
    ID3D11SamplerState *m_pSkyDomeSampler;

    // State blocks
    ID3D11RasterizerState *m_pRsStateSolid;
    ID3D11RasterizerState *m_pRsStateWireframe;
};

