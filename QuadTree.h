#pragma once

#include <vector>

#include <omp.h>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Terrain.h"
#include "Frustum.h"
#include "TerrainShader.h"
#include "LineShader.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// maximum number of node children (4 for quad tree obviously...)
const int MAX_CHILDREN = 4;
const int NUM_CUBE_VERTICES = 8;
const int NUM_BOX_INDICES = 24;

const static float cubeVertices[] =
{
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f, 0.5f,  0.5f,
};

const static unsigned long boxEdges[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

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

    // tree node type
    struct NodeType
    {
        Vec3f position;
        float width;

        int triangleCount;

        vector<int> vVertexList;

        ID3D11Buffer *pVertexBuffer;
        ID3D11Buffer *pIndexBuffer;

        ID3D11Buffer *pCubeVertexBuffer;

        NodeType *pChildNodes[MAX_CHILDREN];
    };

public:
    QuadTree();
    QuadTree(const QuadTree &);
    ~QuadTree();

    bool Initialize(ID3D11Device *pDevice,
                    int numCpu);
    void Shutdown();
    // Release tree nodes.
    void ClearTree();

    // Build the quad tree
    bool BuildTree(Terrain *pTerrain,
                   ID3D11Device *pDevice,
                   const int maxTriangles,
                   bool enabled);

    // Render nodes.
    void Render(Frustum *pFrustum,
                ID3D11DeviceContext *pContext,
                TerrainShader *pShader,
                bool wireframe);

    // Render node border boxes.
    void RenderBorder(Frustum *pFrustum,
                      ID3D11DeviceContext *pContext,
                      LineShader *pShader);

    // Returns the number of currently rendered triangles .
    int GetDrawCount();

    // Get the height of the terrain triangle at a certain position on the plain.
    bool GetHeightAtPosition(float posX, float posZ, float &height);

private:

    template<typename T> void SafeRelease(T *&obj)
    {
        if (obj)
        {
            obj->Release();
            obj = NULL;
        }
    }

    // calculate center point and width of the mesh
    void CalculateMeshDimensions(int vertexCount, Vec3f &center, float &width);

    // create a tree node
    void CreateTreeNode(NodeType *pNode,
                        Vec3f center,
                        float width,
                        int numTriangles,
                        ID3D11Device *pDevice);

    // count number of triangles in node
    int CountTriangles(Vec3f position, float width);

    // check if a triangle is contained in the node
    bool IsTriangleContained(int index, Vec3f position, float width);

    // Release node objects.
    void ReleaseNode(NodeType *pNode);

    // Recursive node rendering method with view frustum culling.
    void RenderNode(NodeType *pNode,
                    Frustum *pFrustum,
                    ID3D11DeviceContext *pContext,
                    TerrainShader *pShader,
                    bool wireframe);

    // Recursively render the box frames of nodes.
    void RenderNodeBorder(NodeType *pNode,
                          Frustum *pFrustum,
                          ID3D11DeviceContext *pContext,
                          LineShader *pLineShader);

    // Find the node containing a specific position and
    // returns the height of the triangle at that position.
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
    // members
    int m_triangleCount;
    int m_drawCount;
    int m_maxTrianges;
    bool m_quadTreeEnabled;

    vector<Vec3f> m_vVertexPositions;
    vector<Vec3f> m_vVertexTexCoords;
    vector<Vec3f> m_vVertexNormals;
    //vector<Vec4f> m_vVertexColors;

    NodeType *m_pRootNode;

    // Index buffer for the rendering of node frame boxes.
    ID3D11Buffer *m_pCubeIndexBuffer;
};

