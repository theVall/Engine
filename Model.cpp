#include "Model.h"


Model::Model(void)
{
    m_vertexBuffer = 0;
    m_indexBuffer  = 0;
    m_texture = 0;
    m_model = 0;
}


Model::~Model(void)
{
}


bool Model::Initialize(ID3D11Device *device, WCHAR *modelFilename, WCHAR *textureFilename)
{
    bool result;

    result = LoadModel(modelFilename);
    if (!result)
    {
        return false;
    }

    result = InitializeBuffers(device);
    if(!result)
    {
        return false;
    }

    result = LoadTexture(device, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


void Model::Shutdown()
{
    ReleaseModel();
    ReleaseTexture();
    ShutdownBuffers();

    return;
}


void Model::Render(ID3D11DeviceContext* deviceContext)
{
    //  Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return;
}


bool Model::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;
    HRESULT result;

    vertices = new VertexType[m_vertexCount];
    if(!vertices)
    {
        return false;
    }

    indices = new unsigned long[m_indexCount];
    if(!indices)
    {
        return false;
    }

    // Load the vertex array with data.
    for (int i = 0; i < m_vertexCount; ++i)
    {
        vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
        vertices[i].texture  = XMFLOAT2(m_model[i].tu, m_model[i].tv);
        vertices[i].normal   = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

        // Load the index array with data.
        indices[i] = i;
    }


    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the sub resource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if(FAILED(result))
    {
        return false;
    }

    result = SetIndexBuffer(device, indices);
    if (FAILED(result))
    {
        return false;
    }

    delete [] vertices;
    vertices = 0;

    return true;
}


void Model::ShutdownBuffers()
{
    // Release the index buffer.
    if(m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // Release the vertex buffer.
    if(m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}


void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    stride = sizeof(VertexType); 
    offset = 0;
    
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}


bool Model::LoadModel(WCHAR *filename)
{
    ifstream fin;
    char input;
    
    fin.open(filename);
    if (fin.fail())
    {
        return false;
    }

    // Read up to the value of vertex count.
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }

    fin >> m_vertexCount;
    m_indexCount = m_vertexCount;

    m_model = new ModelType[m_vertexCount];
    if (!m_model)
    {
        return false;
    }

    // Read up to the beginning of the data.
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }
    fin.get(input);
    fin.get(input);

    // Read in the vertex data.
    for (int i = 0; i < m_vertexCount; ++i)
    {
        // position
        fin >> m_model[i].x  >> m_model[i].y >> m_model[i].z;
        // texture coordinates
        fin >> m_model[i].tu >> m_model[i].tv;
        // normal
        fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
    }

    fin.close();

    return true;
}


void Model::ReleaseModel()
{
    if (m_model)
    {
        delete[] m_model;
        m_model = 0;
    }

    return;
}