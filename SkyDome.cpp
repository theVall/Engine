#include "SkyDome.h"


SkyDome::SkyDome()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
}


SkyDome::SkyDome(const SkyDome &)
{
}


SkyDome::~SkyDome()
{
}


bool SkyDome::Initialize(ID3D11Device *device, WCHAR *modelFilename, Util *util)
{
    bool result;

    // Load in the sky dome model.
    result = util->LoadModel(modelFilename, m_model, m_indexCount, m_vertexCount);
    if (!result)
    {
        return false;
    }

    // Load the sky dome into a vertex and index buffer for rendering.
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // Set the color at the top of the sky dome.
    m_apexColor = XMFLOAT4(0.05f, 0.1f, 0.5f, 1.0f);
    // Set the color at the center of the sky dome.
    m_centerColor = XMFLOAT4(0.35f, 0.35f, 0.6f, 1.0f);

    return true;
}


void SkyDome::Shutdown()
{
    // Release the vertex and index buffer that were used for rendering the sky dome.
    ShutdownBuffers();

    // Release the sky dome model.
    ReleaseSkyDomeModel();

    return;
}


void SkyDome::Render(ID3D11DeviceContext *deviceContext)
{
    RenderBuffers(deviceContext);

    return;
}


int SkyDome::GetIndexCount()
{
    return m_indexCount;
}


XMFLOAT4 SkyDome::GetApexColor()
{
    return m_apexColor;
}


XMFLOAT4 SkyDome::GetCenterColor()
{
    return m_centerColor;
}


void SkyDome::ReleaseSkyDomeModel()
{
    m_model.clear();

    return;
}


bool SkyDome::InitializeBuffers(ID3D11Device *device)
{
    VertexCombined *vertices;
    unsigned long *indices;

    D3D11_BUFFER_DESC vertexBufferDesc;
    D3D11_BUFFER_DESC indexBufferDesc;

    D3D11_SUBRESOURCE_DATA vertexData;
    D3D11_SUBRESOURCE_DATA indexData;

    HRESULT result;

    // Create the vertex array.
    vertices = new VertexCombined[m_vertexCount];
    if (!vertices)
    {
        return false;
    }

    // Create the index array.
    indices = new unsigned long[m_indexCount];
    if (!indices)
    {
        return false;
    }

    // Load the vertex array and index array with data.
    for (int i = 0; i < m_vertexCount; i++)
    {
        vertices[i].position = XMFLOAT4(m_model[i].position.x,
                                        m_model[i].position.y,
                                        m_model[i].position.z,
                                        1.0);
        // tex coordinates
        vertices[i].tex = XMFLOAT2(m_model[i].texture.x,
                                   m_model[i].texture.y);
        indices[i] = i;
    }

    // Set up the description of the vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexCombined)* m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the sub-resource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long)* m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the sub-resource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}


void SkyDome::ShutdownBuffers()
{
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}


void SkyDome::RenderBuffers(ID3D11DeviceContext *deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexCombined);
    offset = 0;

    // Set the vertex and index buffers to active in the input assembler.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}