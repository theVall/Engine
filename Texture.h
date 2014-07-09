#pragma once

#include <d3d11.h>
#include "DDSTextureLoader.h"

using namespace DirectX;

class Texture
{
public:
    Texture();
    Texture(const Texture &);
    ~Texture();

    // Create texture with SRV and RTV
    bool Create2DTextureAndViews(ID3D11Device *device,
                                 UINT width,
                                 UINT height,
                                 DXGI_FORMAT format);

    // Load texture from DDS file
    bool Texture::LoadFromDDS(ID3D11Device *device, WCHAR *filename);

    void Shutdown();

    // Get the 2D texture.
    ID3D11Texture2D *GetTex2D();

    // Get the shader resource view.
    ID3D11ShaderResourceView *GetSrv();

    // Get the render target view.
    ID3D11RenderTargetView *GetRtv();


private:
    ID3D11Texture2D *m_pTex2D;
    ID3D11ShaderResourceView *m_pSrv;
    ID3D11RenderTargetView *m_pRtv;
};

