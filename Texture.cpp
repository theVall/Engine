#include "Texture.h"


Texture::Texture()
{
    m_texture = nullptr;
}


Texture::Texture(const Texture &)
{
}


Texture::~Texture()
{
}


bool Texture::Initialize(ID3D11Device* device, WCHAR* filename)
{
    HRESULT result;

    ID3D11Resource* tex = 0;

    // Load the texture.
    result = CreateDDSTextureFromFile(device, filename, &tex, &m_texture);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void Texture::Shutdown()
{
    // Release the texture resource.
    if (m_texture)
    {
        m_texture->Release();
        m_texture = 0;
    }

    return;
}


ID3D11ShaderResourceView* Texture::GetTexture()
{
    return m_texture;
}