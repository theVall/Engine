#pragma once

#include "ScreenElement.h"

class Model:
    public ScreenElement
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
    // Methods to create a model from file.
    //
    bool LoadModel(WCHAR *);
    //
    // Methods to release a model safely.
    //
    void ReleaseModel();

private:

    ModelType *m_model;
};

