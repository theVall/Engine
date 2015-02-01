#pragma once

#include <vector>

#include <omp.h>
#define NUM_THREADS 8

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Terrain.h"
#include "Frustum.h"
#include "Terrainshader.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int MAX_CHILDREN = 4;

class QuadTree
{
private:
    // used to pass vertex data to buffer for GPU
    // this has to be a struct with XMMath types for vertex buffer
    struct VertexCombined
    {
        XMFLOAT3 position;
        XMFLOAT3 texture;
        XMFLOAT3 normal;
        XMFLOAT4 color;
    };

    // tree node
    struct NodeType
    {
        Vec3f position;
        float width;

        int triangleCount;

        vector<Vec3f> vVertexList;

        ID3D11Buffer *pVertexBuffer;
        ID3D11Buffer *pIndexBuffer;
        NodeType *pChildNodes[MAX_CHILDREN];
    };

public:
    QuadTree();
    QuadTree(const QuadTree &);
    ~QuadTree();

    bool Initialize(Terrain *pTerrain,
                    ID3D11Device *pDevice,
                    const int maxTriangles,
                    bool enabled);
    void Shutdown();
    void Render(Frustum *pFrustum,
                ID3D11DeviceContext *pContext,
                TerrainShader *pShader,
                bool wireframe);

    int GetDrawCount();
    bool GetHeightAtPosition(float posX, float posZ, float &height);

private:
    void CalculateMeshDimensions(int vertexCount, Vec3f &center, float &width);

    void CreateTreeNode(NodeType *pNode,
                        Vec3f center,
                        float width,
                        ID3D11Device *pDevice);

    int CountTriangles(Vec3f position, float width);
    bool IsTriangleContained(int index, Vec3f position, float width);
    void ReleaseNode(NodeType *pNode);
    void RenderNode(NodeType *pNode,
                    Frustum *pFrustum,
                    ID3D11DeviceContext *pContext,
                    TerrainShader *pShader,
                    bool wireframe);

    void FindNode(NodeType *node, float posX, float posZ, float &height);

    // Checks if the y-parallel line with posX and posZ intersects the triangle
    // (v0, v1, v2). Height of the triangle is returned.
    bool CheckHeightOfTriangle(float posX,
                               float posZ,
                               float &height,
                               Vec3f v0,
                               Vec3f v1,
                               Vec3f v2);

private:
    // member
    int m_triangleCount;
    int m_drawCount;
    int m_maxTrianges;
    bool m_quadTreeEnabled;

    vector<VertexType> m_vVertexList;
    NodeType *m_pParentNode;
};

