#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fstream>

#include "Texture.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Model
{
public:

    Model(void);
    Model(const Model &);
    ~Model(void);

    //
    // Method to initialize a model object.
    //
    // <param> device - A D3D device pointer.
    // <param> modelFileName - File name of 3D model.
    // <param> textureFileName - File name of model texture dds-file.
    //
    bool Initialize(ID3D11Device *, WCHAR *, WCHAR *);
    void Shutdown();
    void Render(ID3D11DeviceContext *);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

private:

    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
        XMFLOAT3 normal;
    };

    struct ModelType
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

    //
    // Method to create a vertex buffer.
    //
    bool InitializeBuffers(ID3D11Device *);
    //
    // Method to render a vertex buffer.
    //
    void RenderBuffers(ID3D11DeviceContext *);
    //
    // Methods to safely delete a vertex buffer.
    //
    void ShutdownBuffers();

    //
    // Methods to a texture from file.
    //
    bool LoadTexture(ID3D11Device *, WCHAR *);
    //
    // Methods to release a texture safely.
    //
    void ReleaseTexture();

    //
    // Methods to create a model from file.
    //
    bool LoadModel(WCHAR *);
    //
    // Methods to release a model safely.
    //
    void ReleaseModel();

private:

    ID3D11Buffer *m_vertexBuffer;
    ID3D11Buffer *m_indexBuffer;
    int m_vertexCount;
    int m_indexCount;

    Texture *m_texture;
    ModelType *m_model;
};

