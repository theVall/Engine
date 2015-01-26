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

    struct PerFrameBufferTypeVS
    {
        float heightMapDim;
        float xScale;
        float yScale;
        float padding;
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
                bool wireframe,
                Vec2f upperLeft,
                Vec2f lowerRight);

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
                             ID3D11ShaderResourceView *pHeightSrv,
                             const Vec2f upperLeft,
                             const Vec2f lowerRight);

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
    ID3D11Buffer *m_perFameBufferVS;
};

