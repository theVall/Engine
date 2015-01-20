#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <omp.h>
#define NUM_THREADS 8

#include <vector>
#include <random>
#include <time.h>

#include "Util.h"
#include "Texture.h"
#include "Vec3f.h"
#include "Vec4f.h"
#include "VertexType.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class Terrain
{
public:
    Terrain();
    Terrain(const Terrain &);
    ~Terrain();

    // generate a fractal terrain with the diamond square algorithm (plasma terrain)
    bool GenerateDiamondSquare(Util  *util,
                               int terrainSizeFactor,
                               float hurst,
                               float initialVariance,
                               float scaling,
                               float heightScaling);

    // generate a terrain from a height map image file
    bool GenerateFromFile(Util  *util,
                          WCHAR *heightmapFilename);

    bool Initialize();
    void Shutdown();

    int GetVertexCount();
    vector<VertexType> Terrain::GetVertices();
    ID3D11ShaderResourceView *GetTexture();
    float GetScalingFactor();
    int GetWidth();
    // Set the scaling for the terrain height.
    void SetScalingFactor(float scaling);
    // generate new random variable for fractal terrain generation
    void GenNewRand();
    int GetRand();

private:
    bool InitializeBuffers();
    void ShutdownBuffers();

    // Height map
    bool LoadHeightMap(WCHAR *heightmapFilename);

    // Diamond square
    bool BuildTerrainDiamondSquare(int terrainSizeFactor,
                                   float hurst,
                                   float initialVariance);
    // Interpolate the height values of the points in von-Neumann-neighborhood
    // border treatment: assume 0.0 for outlying points
    void InterpolateHightValues(int index, int divSegment, int idWidth, float &height);

    // Initialization helper methods
    bool LoadColorMap(WCHAR *colorMapFilename);
    void NormalizeHeightMap();
    bool CalculateNormals();
    void ShutdownHeightMap();

    void CalculateTextureCoordinates();

private:
    int m_terrainWidth;
    int m_terrainHeight;
    float m_scaling;
    float m_heightScaling;
    int m_vertexCount;

    int m_rand;

    Util *m_Util;

    vector<VertexType> m_heightMap;
    vector<VertexType> m_vertices;
};
