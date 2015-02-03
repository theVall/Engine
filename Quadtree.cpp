#include "QuadTree.h"


QuadTree::QuadTree()
{
    m_pRootNode = nullptr;
    omp_set_num_threads(NUM_THREADS);

    m_vVertexPositions = new vector<Vec3f>;
    m_vVertexTexCoords = new vector<Vec3f>;
    m_vVertexNormals = new vector<Vec3f>;
    m_vVertexColors = new vector<Vec4f>;

}


QuadTree::QuadTree(const QuadTree &)
{
}


QuadTree::~QuadTree()
{
}


bool QuadTree::Initialize(Terrain *pTerrain,
                          ID3D11Device *pDevice,
                          const int maxTriangles,
                          bool enabled)
{
    // Set the maximum number of triangles per node.
    m_maxTrianges = maxTriangles;
    m_quadTreeEnabled = enabled;

    int vertexCount = pTerrain->GetVertexCount();
    m_triangleCount = vertexCount / 3;

    // get the transposed vertex data
    pTerrain->GetPositions(*m_vVertexPositions);
    pTerrain->GetTexCoords(*m_vVertexTexCoords);
    pTerrain->GetNormals(*m_vVertexNormals);
    pTerrain->GetColors(*m_vVertexColors);

    Vec3f center = Vec3f(0.0f);
    float width;

    CalculateMeshDimensions(vertexCount, center, width);

    // Create the root node for the quad tree.
    m_pRootNode = new NodeType;
    if (!m_pRootNode)
    {
        return false;
    }

    // Recursively build the quad tree based on the vertex list data and mesh dimensions.
    CreateTreeNode(m_pRootNode, center, width, pDevice);

    return true;
}


void QuadTree::Shutdown()
{
    // Recursively release the quad tree node data.
    if (m_pRootNode)
    {
        ReleaseNode(m_pRootNode);
        delete m_pRootNode;
        m_pRootNode = 0;
    }

    return;
}


void QuadTree::Render(Frustum *pFrustum,
                      ID3D11DeviceContext *pContext,
                      TerrainShader *pShader,
                      bool wireframe)
{
    // Reset triangle draw count.
    m_drawCount = 0;

    // Render all visible nodes.
    RenderNode(m_pRootNode, pFrustum, pContext, pShader, wireframe);

    return;
}


int QuadTree::GetDrawCount()
{
    return m_drawCount;
}


void QuadTree::CalculateMeshDimensions(int vertexCount,
                                       Vec3f &center,
                                       float &meshWidth)
{
    // Sum of the positions of all the vertices in the mesh.
    for (int i = 0; i < vertexCount - 1; i++)
    {
        center.x += m_vVertexPositions->at(i).x;
        center.z += m_vVertexPositions->at(i).z;
    }
    // Divide by the number of vertices to find the mid-point.
    center.x = center.x / (float) vertexCount;
    center.z = center.z / (float) vertexCount;

    float maxWidth = 0.0f;
    float maxDepth = 0.0f;

    float minWidth = fabsf(m_vVertexPositions->at(0).x - center.x);
    float minDepth = fabsf(m_vVertexPositions->at(0).z - center.z);

    float width;
    float depth;

    // Find maximum and minimum width and depth of the mesh.
    for (int i = 0; i < vertexCount; i++)
    {
        width = fabsf(m_vVertexPositions->at(i).x - center.x);
        depth = fabsf(m_vVertexPositions->at(i).z - center.z);

        if (width > maxWidth)
        {
            maxWidth = width;
        }
        if (depth > maxDepth)
        {
            maxDepth = depth;
        }
        if (width < minWidth)
        {
            minWidth = width;
        }
        if (depth < minDepth)
        {
            minDepth = depth;
        }
    }

    // Find absolute maximum value between min and max depth and width.
    float maxX = (float) max(fabs(minWidth), fabs(maxWidth));
    float maxZ = (float) max(fabs(minDepth), fabs(maxDepth));

    // Calculate the maximum diameter of the mesh.
    meshWidth = max(maxX, maxZ) * 2.0f;

    return;
}


void QuadTree::CreateTreeNode(NodeType *pNode,
                              Vec3f position,
                              float width,
                              ID3D11Device *pDevice)
{
    // set node parameters
    pNode->position = position;
    pNode->width = width;
    pNode->triangleCount = 0;
    pNode->pVertexBuffer = nullptr;
    pNode->pIndexBuffer = nullptr;

    // initialize children
    pNode->pChildNodes[0] = nullptr;
    pNode->pChildNodes[1] = nullptr;
    pNode->pChildNodes[2] = nullptr;
    pNode->pChildNodes[3] = nullptr;

    // Count the number of triangles inside this node.
    int numTriangles = CountTriangles(position, width);
    // if quad tree is disabled: put all triangles in root node
    if (!m_quadTreeEnabled)
    {
        numTriangles = m_triangleCount;
    }

    // return if empty
    if (numTriangles == 0)
    {
        return;
    }
    // Node too big -> create new nodes.
    else if (numTriangles > m_maxTrianges && m_quadTreeEnabled)
    {
        omp_set_num_threads(MAX_CHILDREN);
        #pragma omp parallel for
        for (int i = 0; i < MAX_CHILDREN; i++)
        {
            // Calculate the position offsets for the new child node.
            Vec3f offset = Vec3f(0.0f);
            offset.x = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
            offset.z = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

            // Check if there are any triangles in the new node.
            int count = CountTriangles(position + offset, width / 2.0f);
            if (count > 0)
            {
                pNode->pChildNodes[i] = new NodeType;

                // Extend the tree by adding the child node.
                CreateTreeNode(pNode->pChildNodes[i],
                               position + offset,
                               width / 2.0f,
                               pDevice);
            }
        }
        omp_set_num_threads(NUM_THREADS);

        return;
    }
    // We have a leaf node.
    else
    {
        pNode->triangleCount = numTriangles;
        int vertexCount = numTriangles * 3;

        VertexCombined *pVertices = new VertexCombined[vertexCount];
        unsigned long *pIndices = new unsigned long[vertexCount];

        Vec3f tmpVertex;
        int index = 0;

        for (int i = 0; i < m_triangleCount; ++i)
        {
            // If the triangle is inside this node, add it to the vertex buffer array.
            if (IsTriangleContained(i, position, width))
            {
                for (int j = 0; j < 3; ++j)
                {
                    int vertexId = i * 3 + j;
                    // Get the data of this triangle from the vertex vectors.
                    pVertices[index].position = m_vVertexPositions->at(vertexId).GetAsXMFloat3();
                    pVertices[index].texture = m_vVertexTexCoords->at(vertexId).GetAsXMFloat3();
                    pVertices[index].normal = m_vVertexNormals->at(vertexId).GetAsXMFloat3();
                    pVertices[index].color = m_vVertexColors->at(vertexId).GetAsXMFloat4();
                    pIndices[index] = index;
                    // Store the vertex position information in the vertex list for
                    // intersection test (height based movement)
                    pNode->vVertexList.push_back(m_vVertexPositions->at(vertexId));
                    index++;
                }
            }
        }

        // Vertex buffer description.
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof(VertexCombined) * vertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        // Vertex data.
        D3D11_SUBRESOURCE_DATA vertexData;
        vertexData.pSysMem = pVertices;
        vertexData.SysMemPitch = 0;
        vertexData.SysMemSlicePitch = 0;

        pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &pNode->pVertexBuffer);

        // Index buffer description.
        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;
        indexBufferDesc.StructureByteStride = 0;

        // Index buffer.
        D3D11_SUBRESOURCE_DATA indexData;
        indexData.pSysMem = pIndices;
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pNode->pIndexBuffer);

        // clean-up
        delete[] pVertices;
        pVertices = 0;

        delete[] pIndices;
        pIndices = 0;

        return;
    }
}


int QuadTree::CountTriangles(Vec3f position, float width)
{
    int count = 0;
    int i;
    bool result;

    // parallel section
    #pragma omp parallel private(i, result) shared(count, position, width)
    {
        #pragma omp for
        for (i = 0; i < m_triangleCount; ++i)
        {
            result = IsTriangleContained(i, position, width);
            if (result == true)
            {
                #pragma omp atomic
                count++;
            }
        }
    }

    return count;
}


bool QuadTree::IsTriangleContained(int index, Vec3f position, float width)
{
    float radius = width / 2.0f;

    // Map the index.
    int vertexId = index * 3;

    // Get the vertices of the triangle.
    Vec3f xCoord;
    Vec3f zCoord;
    xCoord.x = m_vVertexPositions->at(vertexId).x;
    zCoord.x = m_vVertexPositions->at(vertexId).z;
    vertexId++;

    xCoord.y = m_vVertexPositions->at(vertexId).x;
    zCoord.y = m_vVertexPositions->at(vertexId).z;
    vertexId++;

    xCoord.z = m_vVertexPositions->at(vertexId).x;
    zCoord.z = m_vVertexPositions->at(vertexId).z;

    // Min and max of x-coordinate inside the triangle?
    float minimumX = min(xCoord.x, min(xCoord.y, xCoord.z));
    if (minimumX > (position.x + radius))
    {
        return false;
    }
    float maximumX = max(xCoord.x, max(xCoord.y, xCoord.z));
    if (maximumX < (position.x - radius))
    {
        return false;
    }

    // Min and max of z-coordinate inside the triangle?
    float minimumZ = min(zCoord.x, min(zCoord.y, zCoord.z));
    if (minimumZ > (position.z + radius))
    {
        return false;
    }
    float maximumZ = max(zCoord.x, max(zCoord.y, zCoord.z));
    if (maximumZ < (position.z - radius))
    {
        return false;
    }

    return true;
}


void QuadTree::ReleaseNode(NodeType *pNode)
{
    // Recursively release the tree nodes.
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        if (pNode->pChildNodes[i] != 0)
        {
            ReleaseNode(pNode->pChildNodes[i]);
        }
    }

    if (pNode->pVertexBuffer)
    {
        pNode->pVertexBuffer->Release();
        pNode->pVertexBuffer = 0;
    }

    if (pNode->pIndexBuffer)
    {
        pNode->pIndexBuffer->Release();
        pNode->pIndexBuffer = 0;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        if (pNode->pChildNodes[i])
        {
            delete pNode->pChildNodes[i];
            pNode->pChildNodes[i] = 0;
        }
    }

    return;
}


void QuadTree::RenderNode(NodeType *pNode,
                          Frustum *pFrustum,
                          ID3D11DeviceContext *pContext,
                          TerrainShader *pShader,
                          bool wireframe)
{
    bool result;

    // Check to see if the node can be seen.
    // __NOTE__ y-Center value is assumed to be 0! (only 2D quad tree)
    // -> Undefined behavior for (much) "higher/lower" nodes.
    result = pFrustum->CheckCube(pNode->position, pNode->width / 2.0f);
    if (!result)
    {
        return;
    }

    int count = 0;
    for (int i = 0; i < MAX_CHILDREN; ++i)
    {
        if (pNode->pChildNodes[i] != 0)
        {
            count++;
            RenderNode(pNode->pChildNodes[i], pFrustum, pContext, pShader, wireframe);
        }
    }
    if (count != 0)
    {
        return;
    }

    unsigned int stride = sizeof(VertexType);
    unsigned int offset = 0;

    // Set the vertex and index buffers to active in the input assembler.
    pContext->IASetVertexBuffers(0, 1, &pNode->pVertexBuffer, &stride, &offset);
    pContext->IASetIndexBuffer(pNode->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Number of indices is trice the triangle amount.
    int indexCount = pNode->triangleCount * 3;

    // The actual render call.
    pShader->RenderShader(pContext, indexCount, wireframe);

    // Update the number of triangles to contain the newly rendered ones.
    m_drawCount += pNode->triangleCount;

    return;
}


bool QuadTree::GetHeightAtPosition(float posX, float posZ, float &height)
{
    float meshMinX = m_pRootNode->position.x - (m_pRootNode->width / 2.0f);
    float meshMaxX = m_pRootNode->position.x + (m_pRootNode->width / 2.0f);

    float meshMinZ = m_pRootNode->position.z - (m_pRootNode->width / 2.0f);
    float meshMaxZ = m_pRootNode->position.z + (m_pRootNode->width / 2.0f);

    // Make sure the coordinates are actually above a polygon.
    if ((posX < meshMinX) || (posX > meshMaxX) || (posZ < meshMinZ) || (posZ > meshMaxZ))
    {
        return false;
    }

    // Find the node which contains the polygon for this position.
    FindNode(m_pRootNode, posX, posZ, height);

    return true;
}


void QuadTree::FindNode(NodeType *pNode, float x, float z, float &height)
{
    // triangle vertices
    Vec3f v0;
    Vec3f v1;
    Vec3f v2;

    bool foundHeight = false;

    // Calculate the dimensions of this node.
    float xMin = pNode->position.x - (pNode->width / 2.0f);
    float xMax = pNode->position.x + (pNode->width / 2.0f);

    float zMin = pNode->position.z - (pNode->width / 2.0f);
    float zMax = pNode->position.z + (pNode->width / 2.0f);

    // Check if the x and z coordinate are inside this node.
    if ((x < xMin) || (x > xMax) || (z < zMin) || (z > zMax))
    {
        return;
    }

    // Check nodes to see if child nodes exist.
    int count = 0;
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        if (pNode->pChildNodes[i] != 0)
        {
            count++;
            // recursive call
            FindNode(pNode->pChildNodes[i], x, z, height);
        }
    }

    // If there are child nodes, return. The polygon will be in one of these.
    if (count > 0)
    {
        return;
    }

    // If there are no children, the polygon must be in this node.
    // Check polygons in this node to find the height.
    for (int i = 0; i < pNode->triangleCount; i += 3)
    {
        v0 = pNode->vVertexList.at(i);
        v1 = pNode->vVertexList.at(i + 1);
        v2 = pNode->vVertexList.at(i + 2);

        // Check if this is the polygon we are looking for.
        foundHeight = CheckHeightOfTriangle(x, z, height, v0, v1, v2);

        if (foundHeight)
        {
            return;
        }
    }

    return;
}


bool QuadTree::CheckHeightOfTriangle(float x, float z, float &height,
                                     Vec3f v0, Vec3f v1, Vec3f v2)
{
    // Starting position of ray cast.
    Vec3f startVector = Vec3f(x, 0.0f, z);
    // The direction the ray is being cast (straight down).
    Vec3f directionVector = Vec3f(0.0f, -1.0f, 0.0f);

    // Calculate the three edges of the triangle from the given points.
    Vec3f edge1 = v1 - v0;
    Vec3f edge2 = v2 - v0;
    Vec3f normal = normal.GetCross(edge1, edge2);
    Vec3f w0 = startVector - v0;

    float numerator = -normal.Dot(w0);
    float denominator = normal.Dot(directionVector);

    // Prevent division overflow.
    if (fabs(denominator) < 0.000001f)
    {
        return false;
    }

    // Calculate triangle intersection.
    float r = numerator / denominator;
    Vec3f intersectionVec = startVector + directionVector*r;

    // check if intersection occurs inside the triangle
    float uu = edge1.Dot(edge1);
    float uv = edge1.Dot(edge2);
    float vv = edge2.Dot(edge2);

    Vec3f w = intersectionVec - v0;
    float wu = w.Dot(edge1);
    float wv = w.Dot(edge2);
    float D = uv * uv - uu * vv;

    // get and test parametric coordinates
    float s = (uv * wv - vv * wu) / D;
    // Intersection outside the triangle
    if (s < 0.0 || s > 1.0)
    {
        return false;
    }

    float t = (uv * wu - uu * wv) / D;
    // Intersection outside the triangle
    if (t < 0.0 || (s + t) > 1.0)
    {
        return false;
    }

    // intersection inside the triangle
    height = intersectionVec.y;

    return true;
}