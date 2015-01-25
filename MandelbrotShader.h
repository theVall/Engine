#pragma once

#include "ShaderProgram.h"

class MandelbrotShader :
    public ShaderProgram
{
    struct TerrainVertex
    {
        float indexX;
        float indexY;
    };

    struct PerFrameBufferTypePS
    {
        XMFLOAT3 lightDir;
    };

public:
    MandelbrotShader(int meshDim);
    MandelbrotShader(const MandelbrotShader &);
    ~MandelbrotShader();

    bool Render(ID3D11DeviceContext *pContext,
                const XMMATRIX &worldMatrix,
                const XMMATRIX &viewMatrix,
                const XMMATRIX &projectionMatrix,
                const XMFLOAT3 &lightDir,
                ID3D11ShaderResourceView *pHeightSrv,
                bool wireframe);

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
                             const XMFLOAT3 &lightDir,
                             ID3D11ShaderResourceView *pHeightSrv);

    void RenderShader(ID3D11DeviceContext *pContext, bool wireframe);

    // Create vertices for the Mandelbrot terrain surface mesh
    bool CreateSurfaceVertices(ID3D11Device *pDevice);

private:
    // members
    int m_meshDim;
    int m_numIndices;

    ID3D11Buffer *m_pMeshVB;
    ID3D11Buffer *m_pMeshIB;

    // Shaders
    ID3D11VertexShader *m_pMandelbrotVS;
    ID3D11PixelShader *m_pMandelbrotPS;

    // Buffers
    ID3D11Buffer *m_perFameBufferPS;

    // Samplers
    ID3D11SamplerState *m_pHeightSampler;
};

