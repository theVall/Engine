#include "Terrain.h"


Terrain::Terrain()
{
    m_heightMap = 0;
    m_ImageUtil = 0;
    m_Texture = 0;
    m_scaling = 2.0f;
    m_vertices = 0;
}


Terrain::Terrain(const Terrain &)
{
}


Terrain::~Terrain()
{
}


bool Terrain::Initialize(ID3D11Device *device,
                         WCHAR *heightmapFilename,
                         WCHAR *texFilename,
                         ImageUtil *imgUtil)
{
    bool result;
    m_ImageUtil = imgUtil;

    // Load in the height map for the terrain from file.
    result = LoadHeightMap(heightmapFilename);
    if (!result)
    {
        return false;
    }

    // Normalize the height of the height map. Scaling is negative proportional.
    NormalizeHeightMap();

    // Calculate texture coordinates and load the texture from file.
    CalculateTextureCoordinates();
    result = LoadTexture(device, texFilename);
    if (!result)
    {
        return false;
    }

    // Calculate the normals for the terrain data.
    result = CalculateNormals();
    if (!result)
    {
        return false;
    }

    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    return true;
}


void Terrain::Shutdown()
{
    ReleaseTexture();
    ShutdownBuffers();
    ShutdownHeightMap();

    return;
}


ID3D11ShaderResourceView* Terrain::GetTexture()
{
    return m_Texture->GetTexture();
}


float Terrain::GetScalingFactor()
{
    return m_scaling;
}


void Terrain::SetScalingFactor(float scaling)
{
    m_scaling = 1.0f / scaling;
}


bool Terrain::LoadHeightMap(WCHAR *hightmapFilename)
{
    bool result = false;

    int k = 0;
    int index;

    unsigned char *pixelData = nullptr;
    unsigned char height;

    result = m_ImageUtil->LoadBMP(hightmapFilename, pixelData, &m_terrainHeight, &m_terrainWidth);
    if (!result)
    {
        return false;
    }

    // Create the structure to hold the height map data.
    m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
    if (!m_heightMap)
    {
        return false;
    }

    // Read the image data into the height map.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            height = pixelData[k];

            index = (m_terrainHeight * j) + i;

            m_heightMap[index].x = (float)i;
            m_heightMap[index].y = (float)height;
            m_heightMap[index].z = (float)j;

            k += 3;
        }
    }
    // Release the bitmap image data.
    delete[] pixelData;
    pixelData = 0;

    return true;
}


void Terrain::NormalizeHeightMap()
{
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i  <m_terrainWidth; i++)
        {
            m_heightMap[(m_terrainHeight * j) + i].y /= m_scaling;
        }
    }

    return;
}


bool Terrain::CalculateNormals()
{
    int index;
    int index1;
    int index2;
    int index3;

    int count;

    float vertex1[3];
    float vertex2[3];
    float vertex3[3];

    float vector1[3];
    float vector2[3];

    float sum[3];
    float length;

    VectorType* normals;

    // Create a temporary array to hold the non-normalized normal vectors.
    normals = new VectorType[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
    if (!normals)
    {
        return false;
    }

    // Go through all the faces in the mesh and calculate their normals.
    for (int j = 0; j < (m_terrainHeight - 1); j++)
    {
        for (int i = 0; i < (m_terrainWidth - 1); i++)
        {
            index1 = (j * m_terrainHeight) + i;
            index2 = (j * m_terrainHeight) + (i + 1);
            index3 = ((j + 1) * m_terrainHeight) + i;

            // Get three m_vertices from the face.
            vertex1[0] = m_heightMap[index1].x;
            vertex1[1] = m_heightMap[index1].y;
            vertex1[2] = m_heightMap[index1].z;

            vertex2[0] = m_heightMap[index2].x;
            vertex2[1] = m_heightMap[index2].y;
            vertex2[2] = m_heightMap[index2].z;

            vertex3[0] = m_heightMap[index3].x;
            vertex3[1] = m_heightMap[index3].y;
            vertex3[2] = m_heightMap[index3].z;

            // Calculate the two vectors for this face.
            vector1[0] = vertex1[0] - vertex3[0];
            vector1[1] = vertex1[1] - vertex3[1];
            vector1[2] = vertex1[2] - vertex3[2];
            vector2[0] = vertex3[0] - vertex2[0];
            vector2[1] = vertex3[1] - vertex2[1];
            vector2[2] = vertex3[2] - vertex2[2];

            index = (j * (m_terrainHeight - 1)) + i;

            // Calculate the cross product of those two vectors to get the
            // non-normalized value for this face normal.
            normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
            normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
            normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
        }
    }

    // Go through all the vertices and take an average of each face normal 	
    // that the vertex touches to get the averaged normal for that vertex.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            sum[0] = 0.0f;
            sum[1] = 0.0f;
            sum[2] = 0.0f;

            count = 0;

            // Bottom left face.
            if (((i - 1) >= 0) && ((j - 1) >= 0))
            {
                index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);

                sum[0] += normals[index].x;
                sum[1] += normals[index].y;
                sum[2] += normals[index].z;
                count++;
            }

            // Bottom right face.
            if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
            {
                index = ((j - 1) * (m_terrainHeight - 1)) + i;

                sum[0] += normals[index].x;
                sum[1] += normals[index].y;
                sum[2] += normals[index].z;
                count++;
            }

            // Upper left face.
            if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
            {
                index = (j * (m_terrainHeight - 1)) + (i - 1);

                sum[0] += normals[index].x;
                sum[1] += normals[index].y;
                sum[2] += normals[index].z;
                count++;
            }

            // Upper right face.
            if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
            {
                index = (j * (m_terrainHeight - 1)) + i;

                sum[0] += normals[index].x;
                sum[1] += normals[index].y;
                sum[2] += normals[index].z;
                count++;
            }

            // Take the average of the faces touching this vertex.
            sum[0] = (sum[0] / (float)count);
            sum[1] = (sum[1] / (float)count);
            sum[2] = (sum[2] / (float)count);

            // Calculate the length of this normal.
            length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

            // Get an index to the vertex location in the height map array.
            index = (j * m_terrainHeight) + i;

            // Normalize the shared normal for this vertex.
            m_heightMap[index].nx = (sum[0] / length);
            m_heightMap[index].ny = (sum[1] / length);
            m_heightMap[index].nz = (sum[2] / length);
        }
    }

    // Release the temporary normals.
    delete[] normals;
    normals = 0;

    return true;
}


void Terrain::ShutdownHeightMap()
{
    if (m_heightMap)
    {
        delete[] m_heightMap;
        m_heightMap = 0;
    }

    return;
}


void Terrain::CalculateTextureCoordinates()
{
    int incrementCount = 0;
    float incrementValue = 0.0f;

    int tuCount = 0;
    int tvCount = 0;

    float tuCoordinate = 0.0f;
    float tvCoordinate = 1.0f;

    // Calculate how much to increment the texture coordinates by.
    incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

    // Calculate how many times to repeat the texture.
    incrementCount = m_terrainWidth / TEXTURE_REPEAT;

    // Loop through the height map and calculate the texture coordinates for each vertex.
    for (int j = 0; j<m_terrainHeight; j++)
    {
        for (int i = 0; i<m_terrainWidth; i++)
        {
            // Store the texture coordinate in the height map.
            m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
            m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

            tuCoordinate += incrementValue;
            tuCount++;

            // Check if at the far right end of the texture then start at the beginning.
            if (tuCount == incrementCount)
            {
                tuCoordinate = 0.0f;
                tuCount = 0;
            }
        }

        tvCoordinate -= incrementValue;
        tvCount++;

        // Check if at the top of the texture then start at the bottom again.
        if (tvCount == incrementCount)
        {
            tvCoordinate = 1.0f;
            tvCount = 0;
        }
    }

    return;
}


bool Terrain::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
    bool result;

    m_Texture = new Texture;
    if (!m_Texture)
    {
        return false;
    }

    result = m_Texture->Initialize(device, filename);
    if (!result)
    {
        return false;
    }

    return true;
}


void Terrain::ReleaseTexture()
{
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}


bool Terrain::InitializeBuffers(ID3D11Device* device)
{
    int index = 0;

    int index1;
    int index2;
    int index3;
    int index4;

    float tu;
    float tv;

    // Calculate the number of vertices in the terrain mesh.
    m_vertexCount = (m_terrainWidth - 1)*(m_terrainHeight - 1)*6;

    // Create the vertex array.
    m_vertices = new VertexType[m_vertexCount];
    if (!m_vertices)
    {
        return false;
    }

    for (int j = 0; j < (m_terrainHeight - 1); j++)
    {
        for (int i = 0; i < (m_terrainWidth - 1); i++)
        {
            index1 = (m_terrainHeight *       j) +  i;      // Bottom left.
            index2 = (m_terrainHeight *       j) + (i + 1); // Bottom right.
            index3 = (m_terrainHeight * (1 + j)) +  i;      // Upper left.
            index4 = (m_terrainHeight * (1 + j)) + (i + 1); // Upper right.

            // Upper left.
            tv = m_heightMap[index3].tv;

            // Modify the texture coordinates to cover the top edge.
            if (tv == 1.0f)
            {
                tv = 0.0f;
            }

            m_vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
            m_vertices[index].texture = XMFLOAT3(m_heightMap[index3].tu, tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
            index++;

            // Upper right.
            tu = m_heightMap[index4].tu;
            tv = m_heightMap[index4].tv;

            // Modify the texture coordinates to cover the top and right edge.
            if (tu == 0.0f)
            { 
                tu = 1.0f;
            }
            if (tv == 1.0f)
            {
                tv = 0.0f;
            }

            m_vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
            m_vertices[index].texture = XMFLOAT3(tu, tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
            index++;

            // Bottom left.
            m_vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
            m_vertices[index].texture = XMFLOAT3(m_heightMap[index1].tu, m_heightMap[index1].tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
            index++;

            // Bottom left.
            m_vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
            m_vertices[index].texture = XMFLOAT3(m_heightMap[index1].tu, m_heightMap[index1].tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
            index++;

            // Upper right.
            tu = m_heightMap[index4].tu;
            tv = m_heightMap[index4].tv;

            // Modify the texture coordinates to cover the top and right edge.
            if (tu == 0.0f) 
            { 
                tu = 1.0f;
            }
            if (tv == 1.0f) 
            { 
                tv = 0.0f; 
            }

            m_vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
            m_vertices[index].texture = XMFLOAT3(tu, tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
            index++;

            // Bottom right.
            tu = m_heightMap[index2].tu;

            // Modify the texture coordinates to cover the right edge.
            if (tu == 0.0f)
            {
                tu = 1.0f;
            }

            m_vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
            m_vertices[index].texture = XMFLOAT3(tu, m_heightMap[index2].tv, 0.0f);
            m_vertices[index].normal = XMFLOAT3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
            index++;
        }
    }

    return true;
}


void Terrain::ShutdownBuffers()
{
    // Release the vertex array.
    if (m_vertices)
    {
        delete[] m_vertices;
        m_vertices = 0;
    }

    return;
}


int Terrain::GetVertexCount()
{
    return m_vertexCount;
}


void Terrain::CopyVertexArray(void* vertexList)
{
    //vertexList = m_vertices;
    memcpy(vertexList, m_vertices, sizeof(VertexType) * m_vertexCount);
    return;
}