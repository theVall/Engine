#include "Element2d.h"

Element2d::Element2d()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_pTexture = 0;
}


Element2d::Element2d(const Element2d &)
{
}


Element2d::~Element2d()
{
}


bool Element2d::Initialize(ID3D11Device *pDevice,
                           int screenWidth,
                           int screenHeight,
                           WCHAR *pTextureFilename,
                           int elementWidth,
                           int elementHeight)
{
    bool result;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_elementWidth = elementWidth;
    m_elementHeight = elementHeight;

    m_previousPosX = -1;
    m_previousPosY = -1;

    result = InitializeBuffers(pDevice);
    if (!result)
    {
        return false;
    }

    result = LoadTextureFromFile(pDevice, pTextureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

bool Element2d::Initialize(ID3D11Device *pDevice,
                           int screenWidth,
                           int screenHeight,
                           int elementWidth,
                           int elementHeight,
                           ID3D11ShaderResourceView *pSrv)
{
    bool result;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_elementWidth = elementWidth;
    m_elementHeight = elementHeight;

    m_previousPosX = -1;
    m_previousPosY = -1;

    result = InitializeBuffers(pDevice);
    if (!result)
    {
        return false;
    }

    result = LoadTextureFromSrv(pSrv);
    if (!result)
    {
        return false;
    }

    return true;
}


void Element2d::Shutdown()
{
    ReleaseTexture();
    ShutdownBuffers();

    return;
}


bool Element2d::Render(ID3D11DeviceContext *pContext, int positionX, int positionY)
{
    // Re-build the dynamic vertex buffer for rendering to possibly a different
    // location on the screen.
    if (!UpdateBuffers(pContext, positionX, positionY))
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline to prepare for drawing.
    RenderBuffers(pContext);

    return true;
}


bool Element2d::InitializeBuffers(ID3D11Device *pDevice)
{
    VertexType *vertices;
    unsigned long *indices;
    D3D11_BUFFER_DESC vertexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;
    HRESULT result;

    m_vertexCount = 6;
    m_indexCount = m_vertexCount;

    vertices = new VertexType[m_vertexCount];
    if (!vertices)
    {
        return false;
    }

    indices = new unsigned long[m_indexCount];
    if (!indices)
    {
        return false;
    }

    // Initialize vertex array to zeros at first.
    memset(vertices, 0, (sizeof(VertexType)* m_vertexCount));

    // Load the index array with data.
    for (int i = 0; i<m_indexCount; i++)
    {
        indices[i] = i;
    }

    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType)* m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the sub-resource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    result = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    bool bResult = SetIndexBuffer(pDevice, indices);
    if (!bResult)
    {
        return false;
    }

    delete[] vertices;
    vertices = 0;

    return true;
}


void Element2d::ShutdownBuffers()
{
    // Release the index buffer.
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // Release the vertex buffer.
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}


bool Element2d::UpdateBuffers(ID3D11DeviceContext *pContext,
                              int positionX,
                              int positionY)
{
    float left;
    float right;
    float top;
    float bottom;

    VertexType *vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType *verticesPtr;
    HRESULT result;

    // If the position we are rendering this bitmap to has not changed, do not update
    // the vertex buffer since it currently has the correct parameters.
    if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
    {
        return true;
    }
    else
    {
        m_previousPosX = positionX;
        m_previousPosY = positionY;

        // Calculate the screen coordinates.
        left = (float)((m_screenWidth / 2.0f) * -1.0f) + (float)positionX;
        right = left + (float)m_elementWidth;
        top = (float)(m_screenHeight / 2) - (float)positionY;
        bottom = top - (float)m_elementHeight;

        // Create the vertex array.
        vertices = new VertexType[m_vertexCount];
        if (!vertices)
        {
            return false;
        }

        // Load the vertex array with data.
        // First triangle.
        vertices[0].position = XMFLOAT3(left, top, 0.0f);
        vertices[0].texture = XMFLOAT2(0.0f, 0.0f);
        vertices[1].position = XMFLOAT3(right, bottom, 0.0f);
        vertices[1].texture = XMFLOAT2(1.0f, 1.0f);
        vertices[2].position = XMFLOAT3(left, bottom, 0.0f);
        vertices[2].texture = XMFLOAT2(0.0f, 1.0f);
        // Second triangle.
        vertices[3].position = XMFLOAT3(left, top, 0.0f);
        vertices[3].texture = XMFLOAT2(0.0f, 0.0f);
        vertices[4].position = XMFLOAT3(right, top, 0.0f);
        vertices[4].texture = XMFLOAT2(1.0f, 0.0f);
        vertices[5].position = XMFLOAT3(right, bottom, 0.0f);
        vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

        // Lock the vertex buffer.
        result = pContext->Map(m_vertexBuffer,
                               0,
                               D3D11_MAP_WRITE_DISCARD,
                               0,
                               &mappedResource);
        if (FAILED(result))
        {
            return false;
        }

        verticesPtr = (VertexType *)mappedResource.pData;
        memcpy(verticesPtr, (void *)vertices, (sizeof(VertexType)* m_vertexCount));

        // Unlock the vertex buffer.
        pContext->Unmap(m_vertexBuffer, 0);

        delete[] vertices;
        vertices = 0;
    }

    return true;
}


void Element2d::RenderBuffers(ID3D11DeviceContext *pContext)
{
    unsigned int stride;
    unsigned int offset;

    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    pContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    pContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set primitive topology.
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}


void Element2d::SetElementWidth(int newWidth)
{
    m_elementWidth = newWidth;
}


void Element2d::SetElementHeight(int newHeight)
{
    m_elementHeight = newHeight;
}