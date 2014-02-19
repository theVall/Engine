#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "ImageUtil.h"
#include "Texture.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// GLOBALS
const int TEXTURE_REPEAT = 8;

class Terrain
{

public:
    Terrain();
    Terrain(const Terrain &);
    ~Terrain();

    bool Initialize(ID3D11Device *device,
                    WCHAR *hightmapFilename,
                    WCHAR *texFilename,
                    ImageUtil *imageUtil);
    void Shutdown();

    void CopyVertexArray(void*);

    int GetVertexCount();
    ID3D11ShaderResourceView* GetTexture();

    float GetScalingFactor();
    // Set the scaling for the terrain height.
    void SetScalingFactor(float);


private:

    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT3 texture;
        XMFLOAT3 normal;
    };

    struct HeightMapType
    {
        // position
        float x;
        float y;
        float z;
        // texture coordinates
        float tu;
        float tv;
        // normal 
        float nx;
        float ny;
        float nz;
    };

    struct VectorType
    {
        float x;
        float y;
        float z;
    };

    bool InitializeBuffers(ID3D11Device *);
    void ShutdownBuffers();

    // Height map
    bool LoadHeightMap(WCHAR *heightmapFilename);
    void NormalizeHeightMap();
    bool CalculateNormals();
    void ShutdownHeightMap();

    // Texturing
    void CalculateTextureCoordinates();
    bool LoadTexture(ID3D11Device *device, WCHAR *texFilename);
    void ReleaseTexture();

private:

    int m_terrainWidth;
    int m_terrainHeight;

    float m_scaling;

    int m_vertexCount;

    HeightMapType *m_heightMap;
    ImageUtil *m_ImageUtil;
    Texture *m_Texture;

    VertexType* m_vertices;
};
