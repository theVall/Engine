#include "ScreenElement.h"


ScreenElement::ScreenElement(void)
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_pTexture = 0;
}


ScreenElement::ScreenElement(const ScreenElement &)
{
}


ScreenElement::~ScreenElement(void)
{
}


int ScreenElement::GetIndexCount()
{
    return m_indexCount;
}


ID3D11ShaderResourceView *ScreenElement::GetTexture()
{
    return m_pTexture->GetSrv();
}


bool ScreenElement::LoadTextureFromFile(ID3D11Device *pDevice, WCHAR *pFilename)
{
    bool result;

    m_pTexture = new Texture;
    if (!m_pTexture)
    {
        return false;
    }

    result = m_pTexture->LoadFromDDS(pDevice, pFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


bool ScreenElement::LoadTextureFromSrv(ID3D11ShaderResourceView *pSrv)
{
    m_pTexture = new Texture;
    if (!m_pTexture)
    {
        return false;
    }

    m_pTexture->SetSrv(pSrv);

    return true;
}


void ScreenElement::ReleaseTexture()
{
    if (m_pTexture)
    {
        m_pTexture->Shutdown();
        m_pTexture = 0;
    }

    return;
}


bool ScreenElement::SetIndexBuffer(ID3D11Device *pDevice, unsigned long *pIndices)
{
    D3D11_BUFFER_DESC indexBufferDesc;
    D3D11_SUBRESOURCE_DATA indexData;
    HRESULT result;

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long)* m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the sub-resource structure a pointer to the index data.
    indexData.pSysMem = pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    result = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    delete[] pIndices;
    pIndices = 0;

    return true;
}