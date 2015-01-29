#pragma once
#include "ScreenElement.h"
class Element2d :
    public ScreenElement
{
public:
    Element2d();
    Element2d(const Element2d &);
    ~Element2d();

    bool Initialize(ID3D11Device *pDevice,
                    int screenWidth,
                    int screenHeight,
                    WCHAR *pTextureFilename,
                    int bitmapWidth,
                    int bitmapHeight);

    bool Initialize(ID3D11Device *pDevice,
                    int screenWidth,
                    int screenHeight,
                    int bitmapWidth,
                    int bitmapHeight,
                    ID3D11ShaderResourceView *pSrv);

    void Shutdown();

    bool Render(ID3D11DeviceContext *pContext, int positionX, int positionY);

    void SetElementWidth(int width);
    void SetElementHeight(int width);


private:

    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

    bool InitializeBuffers(ID3D11Device *pDevice);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext *pContext, int, int);
    void RenderBuffers(ID3D11DeviceContext *pContext);

private:

    int m_screenWidth;
    int m_screenHeight;

    int m_elementWidth;
    int m_elementHeight;

    int m_previousPosX;
    int m_previousPosY;
};

