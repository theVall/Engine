#include "Font.h"


Font::Font()
{
    m_FontWrapper = 0;
    m_FW1Factory = 0;
}


Font::Font(const Font &)
{
}


Font::~Font()
{
}


bool Font::Initialize(LPCWSTR fontType, ID3D11Device *device)
{
    HRESULT hResult;
    hResult = FW1CreateFactory(FW1_VERSION, &m_FW1Factory);
    if (FAILED(hResult))
    {
        return false;
    }

    bool result = changeFontType(fontType, device);
    if (!result)
    {
        return false;
    }

    return true;
}


void Font::Shutdown()
{
    m_FontWrapper->Release();
    m_FW1Factory->Release();
}


bool Font::changeFontType(LPCWSTR fontType, ID3D11Device *device)
{
    HRESULT hResult = m_FW1Factory->CreateFontWrapper(device, fontType, &m_FontWrapper);
    if (FAILED(hResult))
    {
        return false;
    }

    return true;
}


void Font::drawText(ID3D11DeviceContext *context,
                    WCHAR *text,
                    float fontSize,
                    float xPos,
                    float yPos,
                    int color,
                    int flags)
{
    // Restore device context to the previous after rendering.
    // Must be set for overlay painting.
    flags |= FW1_RESTORESTATE;

    m_FontWrapper->DrawString(context, text, fontSize, xPos, yPos, color, flags);
}