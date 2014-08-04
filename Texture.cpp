#include "Texture.h"


Texture::Texture()
{
    m_pTex2D = nullptr;
    m_pSrv = nullptr;
    m_pRtv = nullptr;
}


Texture::Texture(const Texture &)
{
}


Texture::~Texture()
{
}


bool Texture::Create2DTextureAndViews(ID3D11Device *device,
                                      UINT width,
                                      UINT height,
                                      DXGI_FORMAT format)
{
    HRESULT result;

    // Create 2D texture
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    result = device->CreateTexture2D(&texDesc, NULL, &m_pTex2D);
    if (FAILED(result))
    {
        return false;
    }

    // Create shader resource view
    m_pTex2D->GetDesc(&texDesc);
    //if (m_pSrv)
    //{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    result = device->CreateShaderResourceView(m_pTex2D, &srvDesc, &m_pSrv);
    if (FAILED(result))
    {
        return false;
    }
    //}

    // Create render target view
    //if (m_pRtv)
    //{
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    result = device->CreateRenderTargetView(m_pTex2D, &rtvDesc, &m_pRtv);
    if (FAILED(result))
    {
        return false;
    }
    //}
    return true;
}


bool Texture::LoadFromDDS(ID3D11Device *device, WCHAR *filename)
{
    HRESULT result;

    ID3D11Resource *tex = 0;

    // Load the texture.
    result = CreateDDSTextureFromFile(device, filename, &tex, &m_pSrv);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void Texture::Shutdown()
{
    if (m_pTex2D)
    {
        m_pTex2D->Release();
        m_pTex2D = 0;
    }

    if (m_pSrv)
    {
        m_pSrv->Release();
        m_pSrv = 0;
    }

    if (m_pRtv)
    {
        m_pRtv->Release();
        m_pRtv = 0;
    }

    return;
}


ID3D11Texture2D *Texture::GetTex2D()
{
    return m_pTex2D;
}


ID3D11ShaderResourceView *Texture::GetSrv()
{
    return m_pSrv;
}


ID3D11RenderTargetView *Texture::GetRtv()
{
    return m_pRtv;
}