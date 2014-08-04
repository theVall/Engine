#include "Terrain.h"


Terrain::Terrain()
{
    m_Util = 0;
    m_pTexture = 0;
    m_scaling = 2.0f;
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
                         WCHAR *colorMapFilename,
                         Util  *util)
{
    bool result;
    m_Util = util;

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

    // Load in the color map.
    result = LoadColorMap(colorMapFilename);
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

    result = InitializeBuffers();
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


ID3D11ShaderResourceView *Terrain::GetTexture()
{
    return m_pTexture->GetSrv();
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

    result = m_Util->LoadBMP(hightmapFilename, pixelData, &m_terrainHeight, &m_terrainWidth);
    if (!result)
    {
        return false;
    }

    // Create the structure to hold the height map data.
    m_heightMap.resize(m_terrainWidth * m_terrainHeight);

    // Read the image data into the height map.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            index = (m_terrainHeight * j) + i;

            m_heightMap[index].position = Vec3f((float)i, (float)pixelData[k], (float)j);

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
            m_heightMap[(m_terrainHeight * j) + i].position.y /= m_scaling;
        }
    }

    return;
}


bool Terrain::CalculateNormals()
{
    int index0;
    int index1;
    int index2;
    int index3;

    int count;

    Vec3f vector0;
    Vec3f vector1;

    Vec3f sum;

    vector<Vec3f> normals((m_terrainHeight - 1) * (m_terrainWidth - 1));

    // Go through all the faces in the mesh and calculate their normals.
    for (int j = 0; j < (m_terrainHeight - 1); j++)
    {
        for (int i = 0; i < (m_terrainWidth - 1); i++)
        {
            index0 = (j       * (m_terrainHeight - 1)) + i;
            index1 = (j       *  m_terrainHeight)      + i;
            index2 = (j       *  m_terrainHeight)      + (i + 1);
            index3 = ((j + 1) *  m_terrainHeight)      + i;

            vector0 = m_heightMap[index1].position - m_heightMap[index3].position;
            vector1 = m_heightMap[index3].position - m_heightMap[index2].position;

            normals[index0] = vector0.GetCross(vector0, vector1);
        }
    }

    // Go through all the vertices and take an average of each face normal
    // that the vertex touches to get the averaged normal for that vertex.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            sum = Vec3f(0.0f, 0.0f, 0.0f);
            count = 0;

            // Bottom left face.
            if (((i - 1) >= 0) && ((j - 1) >= 0))
            {
                index0 = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);
                sum += normals[index0];
                count++;
            }

            // Bottom right face.
            if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
            {
                index0 = ((j - 1) * (m_terrainHeight - 1)) + i;
                sum += normals[index0];
                count++;
            }

            // Upper left face.
            if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
            {
                index0 = (j * (m_terrainHeight - 1)) + (i - 1);
                sum += normals[index0];
                count++;
            }

            // Upper right face.
            if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
            {
                index0 = (j * (m_terrainHeight - 1)) + i;
                sum += normals[index0];
                count++;
            }

            // Take the average of the faces touching this vertex.
            sum /= (float)count;

            // Get an index to the vertex location in the height map array.
            index0 = (j * m_terrainHeight) + i;

            // Normalize the shared normal for this vertex.
            m_heightMap[index0].normal = sum.Normalize();
        }
    }

    // Release the temporary normals.
    normals.clear();

    return true;
}


void Terrain::ShutdownHeightMap()
{
    m_heightMap.clear();
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
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            // Store the texture coordinate in the height map.
            m_heightMap[(m_terrainHeight * j) + i].texture.u = tuCoordinate;
            m_heightMap[(m_terrainHeight * j) + i].texture.v = tvCoordinate;
            // 3rd texture coordinate empty yet
            //m_heightMap[(m_terrainHeight * j) + i].texture.z = 1.0f;

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


bool Terrain::LoadTexture(ID3D11Device *device, WCHAR *filename)
{
    bool result;

    m_pTexture = new Texture;
    if (!m_pTexture)
    {
        return false;
    }

    result = m_pTexture->LoadFromDDS(device, filename);
    if (!result)
    {
        return false;
    }

    return true;
}


void Terrain::ReleaseTexture()
{
    if (m_pTexture)
    {
        m_pTexture->Shutdown();
        delete m_pTexture;
        m_pTexture = 0;
    }

    return;
}


bool Terrain::LoadColorMap(WCHAR *filename)
{
    bool result = false;

    int i = 0;
    int k = 0;
    int index;

    unsigned char *pixelData = nullptr;

    // conversion factor used to convert color values
    float conversionFactor = 255.0f;

    result = m_Util->LoadBMP(filename, pixelData, &m_terrainHeight, &m_terrainWidth);
    if (!result)
    {
        return false;
    }

    // Read the image color data into the struct.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (i = 0; i < m_terrainWidth; i++)
        {
            index = (m_terrainHeight * j) + i;

            m_heightMap[index].color.r = (float)pixelData[k++] / conversionFactor;
            m_heightMap[index].color.g = (float)pixelData[k++] / conversionFactor;
            m_heightMap[index].color.b = (float)pixelData[k++] / conversionFactor;
            m_heightMap[index].color.a = 1.0f;
        }
    }
    // Release the bitmap image data.
    delete[] pixelData;
    pixelData = 0;

    return true;
}


bool Terrain::InitializeBuffers()
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

    // Create the vertex vector.
    m_vertices.resize(m_vertexCount);

    for (int j = 0; j < (m_terrainHeight - 1); j++)
    {
        for (int i = 0; i < (m_terrainWidth - 1); i++)
        {
            index1 = (m_terrainHeight *       j) +  i;      // Bottom left.
            index2 = (m_terrainHeight *       j) + (i + 1); // Bottom right.
            index3 = (m_terrainHeight * (1 + j)) +  i;      // Upper left.
            index4 = (m_terrainHeight * (1 + j)) + (i + 1); // Upper right.

            // Upper left.
            // Modify the texture coordinates to cover the top edge.
            tv = m_heightMap[index3].texture.v;
            if (tv == 1.0f)
            {
                tv = 0.0f;
            }
            m_vertices[index] = m_heightMap[index3];
            m_vertices[index++].texture.v = tv;

            // Upper right.
            // Modify the texture coordinates to cover the top and right edge.
            tu = m_heightMap[index4].texture.u;
            tv = m_heightMap[index4].texture.v;
            if (tu == 0.0f)
            {
                tu = 1.0f;
            }
            if (tv == 1.0f)
            {
                tv = 0.0f;
            }
            m_vertices[index] = m_heightMap[index4];
            m_vertices[index].texture.u = tu;
            m_vertices[index++].texture.v = tv;

            // Bottom left.
            m_vertices[index++] = m_heightMap[index1];

            // Bottom left.
            m_vertices[index++] = m_heightMap[index1];

            // Upper right.
            // Modify the texture coordinates to cover the top and right edge.
            tu = m_heightMap[index4].texture.u;
            tv = m_heightMap[index4].texture.v;
            if (tu == 0.0f)
            {
                tu = 1.0f;
            }
            if (tv == 1.0f)
            {
                tv = 0.0f;
            }
            m_vertices[index] = m_heightMap[index4];
            m_vertices[index].texture.u = tu;
            m_vertices[index++].texture.v = tv;

            // Bottom right.
            // Modify the texture coordinates to cover the right edge.
            tu = m_heightMap[index2].texture.u;
            if (tu == 0.0f)
            {
                tu = 1.0f;
            }
            m_vertices[index] = m_heightMap[index2];
            m_vertices[index++].texture.u = tu;
        }
    }

    return true;
}


void Terrain::ShutdownBuffers()
{
    // Release the vertex array.
    m_vertices.clear();

    return;
}


int Terrain::GetVertexCount()
{
    return m_vertexCount;
}


vector<VertexType> Terrain::GetVertices()
{
    return m_vertices;
}