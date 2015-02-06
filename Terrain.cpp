#include "Terrain.h"


Terrain::Terrain()
{
    m_Util = 0;
    m_scaling = 1.0f;
    m_heightScaling = 1.0f;
    m_rand = 14875;

    // random seed
    srand((int)time(NULL));
    omp_set_num_threads(NUM_THREADS);

    // max resolution * 6 for 2 triangles per grid square
    m_vertexData.ReserveAll(1024 * 1024 * 6);
}


Terrain::Terrain(const Terrain &)
{
}


Terrain::~Terrain()
{
}


bool Terrain::GenerateDiamondSquare(Util *util,
                                    int terrainSizeFactor,
                                    float hurst,
                                    float initialVariance,
                                    float scaling,
                                    float heightScaling)
{
    m_scaling = scaling;
    m_heightScaling = heightScaling;

    if (!m_Util)
    {
        m_Util = util;
    }

    // generate the height-map with the diamond-squares algorithm
    if (!BuildTerrainDiamondSquare(terrainSizeFactor, hurst, initialVariance))
    {
        return false;
    }

    // Calculate texture coordinates and load the textures from file.
    CalculateTextureCoordinates();

    if (!Initialize())
    {
        return false;
    }

    return true;
}


bool Terrain::GenerateFromFile(Util *util, WCHAR *heightmapFilename)
{
    if (!m_Util)
    {
        m_Util = util;
    }

    // Load in the height-map for the terrain from file.
    if (!LoadHeightMap(heightmapFilename))
    {
        return false;
    }

    // Calculate texture coordinates and load the textures from file.
    CalculateTextureCoordinates();

    if (!Initialize())
    {
        return false;
    }

    return true;
}


bool Terrain::Initialize()
{
    // Normalize the height of the height map. Scaling is negative proportional.
    NormalizeHeightMap();

    // Calculate the normals for the terrain data.
    if (!CalculateNormals())
    {
        return false;
    }

    // Initialize buffers
    if (!InitializeBuffers())
    {
        return false;
    }

    return true;
}


void Terrain::Shutdown()
{
    m_gridData.ClearAll();
    m_vertexData.ClearAll();

    return;
}


float Terrain::GetScalingFactor()
{
    return m_scaling;
}


void Terrain::SetScalingFactor(float scalingFactor)
{
    m_scaling = 1.0f / scalingFactor;
}


bool Terrain::BuildTerrainDiamondSquare(int terrainSizeFactor,
                                        float hurst,
                                        float initialVariance)
{
    // size and length of the terrain is 2 to the power of the sizeFactor (2^factor)
    // plus 1 for unambiguous center point
    m_terrainHeight = (int)(1 << terrainSizeFactor) + 1;
    if (terrainSizeFactor > 10)
    {

    }

    // terrain is always quadratic
    m_terrainWidth = m_terrainHeight;

    m_gridData.ClearAll();
    m_gridData.ResizeAll(m_terrainWidth * m_terrainHeight);
    m_gridData.ShrinkAll();

    // terrain height offset
    float D = 0.0f;
    float height = 0.0f;
    float variance = initialVariance;

    // random generator
    mt19937 gen(m_rand);
    // generate a normal distribution with initial variance and n = 0
    normal_distribution<float> distr(0.0f, variance*variance);

    // initially set the four corner points with random y-offset
    int index = 0;
    // upper left
    m_gridData.vPostion.at(index) = Vec3f(0.0f);
    // upper right
    index = m_terrainWidth - 1;
    m_gridData.vPostion.at(index) = Vec3f((float)(m_terrainWidth - 1),
                                          distr(gen),
                                          0.0f);
    // lower left
    index = m_terrainWidth*(m_terrainHeight - 1);
    m_gridData.vPostion.at(index) = Vec3f(0.0f,
                                          distr(gen),
                                          (float)(m_terrainHeight - 1));
    // lower right
    index = m_terrainWidth*m_terrainHeight - 1;
    m_gridData.vPostion.at(index) = Vec3f((float)(m_terrainHeight - 1),
                                          distr(gen),
                                          (float)(m_terrainWidth - 1));

    int idWidth = m_terrainWidth;

    for (int i = 0; i < terrainSizeFactor; ++i)
    {
        // calculate division segment length
        int divSegment = m_terrainWidth / (2 * (1 << i));

        // calculate new variance
        variance = (initialVariance*initialVariance) / pow(2.0f, (i + 1) * hurst);
        // generate normal distribution with new variance
        normal_distribution<float> distr(0.0f, variance);

        int modDiv = (1 << i)*2;

        // first step: generate diamonds: 2^n * 2^n midpoints
        // parallel section
        #pragma omp parallel for
        for (int j = 0; j < ((1 << i) * (1 << i)); ++j)
        {
            // index calculation: first part is the x-offset, second part is the y-offset
            int k = j / (1 << i);
            // odd distances horizontal and vertical
            index = divSegment*((2 * j + 1) % modDiv) + divSegment*idWidth*((2 * k + 1) % modDiv);

            // interpolate between neighboring corner points
            height = 0.0f;
            // upper left
            int tmpIndex = index - (divSegment + divSegment*idWidth);
            height += m_gridData.vPostion.at(tmpIndex).y;
            // upper right
            tmpIndex += 2 * divSegment;
            height += m_gridData.vPostion.at(tmpIndex).y;
            // lower right
            tmpIndex = index + (divSegment + divSegment*idWidth);
            height += m_gridData.vPostion.at(tmpIndex).y;
            // lower left
            tmpIndex -= 2 * divSegment;
            height += m_gridData.vPostion.at(tmpIndex).y;
            // normalize
            height /= 4.0f;
#undef min
            // generate random value in range [0,1]
            D = distr(gen);

            m_gridData.vPostion.at(index) = Vec3f((float)(index % m_terrainWidth),
                                                  height + D,
                                                  (float)(index / m_terrainHeight));
        }

        // Second step: generate squares.
        // Divide into two parts: even and odd rows
        // even rows 0, 2, 4, ...
        // parallel section
        #pragma omp parallel for
        for (int j = 0; j < ((1 << i) * ((1 << i) + 1)); ++j)
        {
            // index calculation: first part is the x-offset, second part is the y-offset
            int k = j / (1 << i);
            // odd distances horizontally, even distances vertically
            index = divSegment*((2 * j + 1) % modDiv) + divSegment*(k * 2)*idWidth;

            InterpolateHightValues(index, divSegment, idWidth, height);

            // generate random value in range [0,1]
            D = distr(gen);

            m_gridData.vPostion.at(index) = Vec3f((float)(index % m_terrainWidth),
                                                  height + D,
                                                  (float)(index / m_terrainHeight));
        }

        // odd rows 1, 3, 5,...
        // parallel section
        #pragma omp parallel for
        for (int j = 0; j < ((1 << i) * ((1 << i) + 1)); ++j)
        {
            // index calculation: first part is the x-offset, second part is the y-offset
            int k = j / ((1 << i) + 1);
            // even distances horizontally, odd distances vertically
            index = divSegment*((j  * 2) % (modDiv + 2)) + divSegment*(2 * k + 1)*idWidth;

            height = 0.0f;
            InterpolateHightValues(index, divSegment, idWidth, height);

            // generate random value in range [0,1]
            D = distr(gen);

            m_gridData.vPostion.at(index) = Vec3f((float)(index % m_terrainWidth),
                                                  height + D,
                                                  (float)(index / m_terrainHeight));
        }
    }

    return true;
}


void Terrain::InterpolateHightValues(int index, int divSegment, int idWidth, float &height)
{
    // interpolate between neighboring points (von Neumann neighborhood)
    // special cases: border points
    // => mirrored repeat, assume the value of the opposite neighbor
    height = 0.0f;
    int tmpIndex = 0;

    // upper neighbor
    tmpIndex = index - (divSegment*idWidth);;
    if (tmpIndex < 0)
    {
        tmpIndex = index + (divSegment*idWidth);
    }
    height += m_gridData.vPostion.at(tmpIndex).y;

    // lower neighbor
    tmpIndex = index + (divSegment*idWidth);
    if (tmpIndex >= idWidth * idWidth)
    {
        tmpIndex = index - (divSegment*idWidth);;
    }
    height += m_gridData.vPostion.at(tmpIndex).y;

    // right neighbor
    if (!((index % idWidth) == (idWidth - 1)))
    {
        tmpIndex = index + divSegment;
    }
    else
    {
        tmpIndex = index - divSegment;
    }
    height += m_gridData.vPostion.at(tmpIndex).y;

    // left neighbor
    if (!(index % idWidth) == 0)
    {
        tmpIndex = index - divSegment;
    }
    else
    {
        tmpIndex = index + divSegment;
    }
    height += m_gridData.vPostion.at(tmpIndex).y;

    // normalize
    height /= 4.0f;
}


bool Terrain::LoadHeightMap(WCHAR *hightmapFilename)
{
    int k = 0;
    int index;

    unsigned char *pixelData = nullptr;

    if (!m_Util->LoadBMP(hightmapFilename, pixelData, &m_terrainHeight, &m_terrainWidth))
    {
        return false;
    }

    // Create the structure to hold the height map data.
    m_gridData.ResizeAll(m_terrainWidth * m_terrainHeight);

    // Read the image data into the height map.
    // parallel section
    #pragma omp parallel for
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            index = (m_terrainHeight * j) + i;
            m_gridData.vPostion.at(index) = Vec3f((float)i,
                                                  (float)pixelData[k],
                                                  (float)j);
        }
    }

    // Release the bitmap image data.
    delete[] pixelData;
    pixelData = 0;

    return true;
}


void Terrain::NormalizeHeightMap()
{
    // parallel section
    #pragma omp parallel for
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i  <m_terrainWidth; i++)
        {
            m_gridData.vPostion.at((m_terrainHeight * j) + i) *= m_scaling;
            m_gridData.vPostion.at((m_terrainHeight * j) + i).y *= m_heightScaling;
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
    // parallel section
    #pragma omp parallel for
    for (int j = 0; j < (m_terrainHeight - 1); j++)
    {
        for (int i = 0; i < (m_terrainWidth - 1); i++)
        {
            index0 = (j       * (m_terrainHeight - 1)) + i;
            index1 = (j       *  m_terrainHeight)      + i;
            index2 = (j       *  m_terrainHeight)      + (i + 1);
            index3 = ((j + 1) *  m_terrainHeight)      + i;

            vector0 = m_gridData.vPostion.at(index1) - m_gridData.vPostion.at(index3);
            vector1 = m_gridData.vPostion.at(index3) - m_gridData.vPostion.at(index2);

            normals[index0] = vector0.GetCross(vector0, vector1);
        }
    }

    // Go through all the vertices and take an average of each face normal
    // that the vertex touches to get the averaged normal for that vertex.
    // parallel section
    #pragma omp parallel for
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
            m_gridData.vNormals.at(index0) = sum.Normalize();
        }
    }


    // Release the temporary normals.
    normals.clear();

    return true;
}


void Terrain::CalculateTextureCoordinates()
{
    int tuCount = 0;
    int tvCount = 0;

    Vec2f texCoord = Vec2f(0.0f, 1.0f);
    int textureRepeat = (m_terrainWidth - 1) / 8;

    // Calculate how much to increment the texture coordinates by.
    float incrementValue = (float)textureRepeat / (float)(m_terrainWidth - 1);

    // Calculate how many times to repeat the texture.
    int incrementCount = (m_terrainWidth - 1) / textureRepeat;

    // Loop through the height map and calculate the texture coordinates for each vertex.
    for (int j = 0; j < m_terrainHeight - 1; j++)
    {
        for (int i = 0; i < m_terrainWidth - 1; i++)
        {
            // Store the texture coordinate in the height map.
            m_gridData.vTexCoords.at((m_terrainHeight * j) + i) = Vec3f(texCoord.u,
                                                                        texCoord.v,
                                                                        0.0f);
            // 3rd texture coordinate empty yet
            texCoord.u += incrementValue;
            tuCount++;

            // Check if at the far right end of the texture then start at the beginning.
            if (tuCount == incrementCount)
            {
                texCoord.u = 0.0f;
                tuCount = 0;
            }
        }

        texCoord.v -= incrementValue;
        tvCount++;

        // Check if at the top of the texture then start at the bottom again.
        if (tvCount == incrementCount)
        {
            texCoord.v = 1.0f;
            tvCount = 0;
        }
    }

    return;
}


bool Terrain::LoadColorMap(WCHAR *filename)
{
    bool result = false;

    int i = 0;
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
    #pragma omp parallel for
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (i = 0; i < m_terrainWidth; i++)
        {
            index = (m_terrainHeight * j) + i;

            Vec4f color = Vec4f((float)pixelData[3 * i],
                                (float)pixelData[3 * i + 1],
                                (float)pixelData[3 * i + 2],
                                conversionFactor);

            //m_gridData.vColors.at(index) = color / conversionFactor;
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


    // Calculate the number of vertices in the terrain mesh.
    // Consider a grid of squares (resolution-1 per dimension).
    // Each square becomes two triangles in the  triangle list, so we need
    // 2*3=6 times the number of squares.
    m_vertexCount = (m_terrainWidth - 1)*(m_terrainHeight - 1)*6;

    // Clear and resize the vertex data vectors.
    m_vertexData.ResizeAll(m_vertexCount);
    m_vertexData.ShrinkAll();

    #pragma omp parallel for
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
            Vec2f texCoord = m_gridData.vTexCoords.at(index3).uv;
            texCoord.v = (texCoord.v == 1.0f) ? 0.0f : texCoord.v;

            m_vertexData.TransferData(&m_gridData, index, index3);
            m_vertexData.vTexCoords.at(index++).v = texCoord.v;

            // Upper right.
            // Modify the texture coordinates to cover the top and right edge.
            texCoord = m_gridData.vTexCoords.at(index4).uv;

            texCoord.u = (texCoord.u == 0.0f) ? 1.0f : texCoord.u;
            texCoord.v = (texCoord.v == 1.0f) ? 0.0f : texCoord.v;

            m_vertexData.TransferData(&m_gridData, index, index4);
            m_vertexData.vTexCoords.at(index).u = texCoord.u;
            m_vertexData.vTexCoords.at(index++).v = texCoord.v;

            // Bottom left.
            m_vertexData.TransferData(&m_gridData, index++, index1);

            // Bottom left.
            m_vertexData.TransferData(&m_gridData, index++, index1);

            // Upper right.
            // Modify the texture coordinates to cover the top and right edge.
            texCoord = m_gridData.vTexCoords.at(index4).uv;
            texCoord.u = (texCoord.u == 0.0f) ? 1.0f : texCoord.u;
            texCoord.v = (texCoord.v == 1.0f) ? 0.0f : texCoord.v;

            m_vertexData.TransferData(&m_gridData, index, index4);
            m_vertexData.vTexCoords.at(index).u = texCoord.u;
            m_vertexData.vTexCoords.at(index++).v = texCoord.v;

            // Bottom right.
            // Modify the texture coordinates to cover the right edge.
            texCoord.u = m_gridData.vTexCoords.at(index2).u;
            texCoord.u = (texCoord.u == 0.0f) ? 1.0f : texCoord.u;

            m_vertexData.TransferData(&m_gridData, index, index2);
            m_vertexData.vTexCoords.at(index++).u = texCoord.u;
        }
    }

    // clear grid data
    //m_gridData.ClearAll();

    return true;
}


int Terrain::GetVertexCount()
{
    return m_vertexCount;
}


int Terrain::GetWidth()
{
    return m_terrainWidth;
}


void Terrain::GenNewRand()
{
    m_rand = rand();
}


int Terrain::GetRand()
{
    return m_rand;
}


void Terrain::GetPositions(vector<Vec3f> &vPostion)
{
    vPostion = m_vertexData.vPostion;
}


void Terrain::GetTexCoords(vector<Vec3f> &vTexCoords)
{
    vTexCoords = m_vertexData.vTexCoords;
}


void Terrain::GetNormals(vector<Vec3f> &vNormals)
{
    vNormals = m_vertexData.vNormals;
}


//void Terrain::GetColors(vector<Vec4f> &vColors)
//{
//    vColors = m_vertexData.vColors;
//}