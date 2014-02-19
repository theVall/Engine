#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fstream>

#include "Texture.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class ScreenElement
{
public:

    ScreenElement(void);
    ScreenElement(const ScreenElement &);
    virtual ~ScreenElement(void);

    virtual void Shutdown() = 0;

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

protected:
    //
    // Methods to load a texture from file.
    //
    bool LoadTexture(ID3D11Device *, WCHAR *);
    //
    // Methods to release a texture safely.
    //
    void ReleaseTexture();
    //
    // Method for setting an index buffer.
    //
    bool SetIndexBuffer(ID3D11Device *, unsigned long *);


private:

    //
    // Methods to safely delete buffers.
    //
    virtual void ShutdownBuffers() = 0;

protected:

    ID3D11Buffer *m_vertexBuffer;
    ID3D11Buffer *m_indexBuffer;

    int m_vertexCount;
    int m_indexCount;

    Texture *m_texture;
};

