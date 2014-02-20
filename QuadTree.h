#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Terrain.h"
#include "Frustum.h"
#include "Terrainshader.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int MAX_TRIANGLES = 10000;
const int MAX_CHILDREN = 4;

class QuadTree
{
private:

    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT3 texture;
        XMFLOAT3 normal;
    };

    struct NodeType
    {
        float positionX;
        float positionZ;
        float width;

        int triangleCount;
        ID3D11Buffer *vertexBuffer;
        ID3D11Buffer *indexBuffer;
        NodeType* nodes[4];
    };

public:
    QuadTree();
    QuadTree(const QuadTree&);
    ~QuadTree();

    bool Initialize(Terrain *, ID3D11Device *);
    void Shutdown();
    void Render(Frustum *, ID3D11DeviceContext *, TerrainShader *);

    int GetDrawCount();

private:
    void CalculateMeshDimensions(int vertexCount,
                                 float &centerX,
                                 float &centerZ,
                                 float &width);

    void CreateTreeNode(NodeType* parent,
                        float centerX,
                        float centerZ,
                        float width,
                        ID3D11Device* device);

    int CountTriangles(float, float, float);
    bool IsTriangleContained(int, float, float, float);
    void ReleaseNode(NodeType*);
    void RenderNode(NodeType*, Frustum*, ID3D11DeviceContext*, TerrainShader*);

// member
private:
    int m_triangleCount;
    int m_drawCount;

    VertexType* m_vertexList;
    NodeType* m_parentNode;
};

