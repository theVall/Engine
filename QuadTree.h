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

const int MAX_TRIANGLES = 1000;
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

    struct NodeType
    {
        float positionX;
        float positionZ;
        float width;

        int triangleCount;

        vector<Vec3f> vertexList;

        ID3D11Buffer *vertexBuffer;
        ID3D11Buffer *indexBuffer;
        NodeType *nodes[MAX_CHILDREN];
    };

public:
    QuadTree();
    QuadTree(const QuadTree &);
    ~QuadTree();

    bool Initialize(Terrain *, ID3D11Device *);
    void Shutdown();
    void Render(Frustum *pFrustum,
                ID3D11DeviceContext *pContext,
                TerrainShader *pShader,
                bool wireframe);

    int GetDrawCount();
    bool GetHeightAtPosition(float posX, float posZ, float &height);

private:
    void CalculateMeshDimensions(int vertexCount,
                                 float &centerX,
                                 float &centerZ,
                                 float &width);

    void CreateTreeNode(NodeType *parent,
                        float centerX,
                        float centerZ,
                        float width,
                        ID3D11Device *device);

    int CountTriangles(float posX, float posY, float width);
    bool IsTriangleContained(int index, float posX, float posZ, float width);
    void ReleaseNode(NodeType *pNode);
    void RenderNode(NodeType *pNode,
                    Frustum *pFrustum,
                    ID3D11DeviceContext *pContext,
                    TerrainShader *pShader,
                    bool wireframe);

    void FindNode(NodeType *node, float posX, float posZ, float &height);

    // Checks if the y-parallel line with posX and posZ intersects the triangle (v0, v1, v2).
    // <param> posX x-position of the point which height is to be determined.
    // <param> posZ z-position of the point which height is to be determined.
    // <returns> height the height value of the triangle on the position.
    bool CheckHeightOfTriangle(float posX, float posZ, float &height, Vec3f v0, Vec3f v1, Vec3f v2);

// member
private:
    int m_triangleCount;
    int m_drawCount;

    vector<VertexType> m_vertexList;
    NodeType *m_pParentNode;
};

