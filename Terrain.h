#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <omp.h>

#include <vector>
#include <random>
#include <time.h>

#include "Util.h"
#include "Texture.h"
#include "Vec2f.h"
#include "Vec3f.h"
#include "Vec4f.h"
#include "VertexType.h"

#define NUM_THREADS 8

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class Terrain
{
    // Transposed approach, all vertex attributes are in separate arrays
    struct VerticesTransposed
    {
        vector<Vec3f> *vPostion;
        vector<Vec3f> *vTexCoords;
        vector<Vec3f> *vNormals;
        vector<Vec4f> *vColors;

        void Init()
        {
            vPostion = new vector<Vec3f>;
            vTexCoords = new vector<Vec3f>;
            vNormals = new vector<Vec3f>;
            vColors = new vector<Vec4f>;
        }

        void ClearAll()
        {
            vPostion->clear();
            vTexCoords->clear();
            vNormals->clear();
            vColors->clear();
        };

        void ResizeAll(size_t size)
        {
            vPostion->resize(size);
            vTexCoords->resize(size);
            vNormals->resize(size);
            vColors->resize(size);
        }

        void TransferData(VerticesTransposed *pOther, int idSelf, int idOther)
        {
            vPostion->at(idSelf) = pOther->vPostion->at(idOther);
            vTexCoords->at(idSelf) = pOther->vTexCoords->at(idOther);
            vNormals->at(idSelf) = pOther->vNormals->at(idOther);
            vColors->at(idSelf) = pOther->vColors->at(idOther);
        }
    };

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
    bool GenerateFromFile(Util *util, WCHAR *heightmapFilename);

    bool Initialize();
    void Shutdown();

    int GetVertexCount();
    ID3D11ShaderResourceView *GetTexture();
    float GetScalingFactor();
    int GetWidth();
    // Set the scaling for the terrain height.
    void SetScalingFactor(float scaling);
    // generate new random variable for fractal terrain generation
    void GenNewRand();
    int GetRand();

    // Get pointers to transposed vertex data
    void GetPositions(vector<Vec3f> &vPositions);
    void GetTexCoords(vector<Vec3f> &vTexCoords);
    void GetNormals(vector<Vec3f> &vNormals);
    void GetColors(vector<Vec4f> &vColors);

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
    // border treatment: mirrored repeat
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

    // Util class pointer for .dds reading function (height-map).
    Util *m_Util;

    // the vertex data of the initially generated grid
    // (from height-map or diamond-square)
    VerticesTransposed m_pGridData;
    // render ready vertex data
    VerticesTransposed m_pVertexData;
};
