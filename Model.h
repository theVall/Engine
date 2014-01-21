#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class Model
{
public:

    Model(void);
    Model(const Model &);
    ~Model(void);

    bool Initialize(ID3D11Device *);
    void Shutdown();
    void Render(ID3D11DeviceContext *);

    int GetIndexCount();

private:

    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    bool InitializeBuffers(ID3D11Device *);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext *);

private:

    ID3D11Buffer *m_vertexBuffer;
    ID3D11Buffer *m_indexBuffer;
    int m_vertexCount;
    int m_indexCount;

};

