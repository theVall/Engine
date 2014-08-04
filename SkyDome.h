#pragma once

#include "ScreenElement.h"
#include "Vec4f.h"
#include "VertexType.h"
#include "Util.h"

#include "vector"

using namespace math;


class SkyDome :
    public ScreenElement
{
private:
    // Buffer ready struct
    struct VertexCombined
    {
        XMFLOAT4 position;
        XMFLOAT2 tex;
    };

public:
    SkyDome();
    SkyDome(const SkyDome &);
    ~SkyDome();

    bool Initialize(ID3D11Device *, WCHAR *modelFilename, Util *m_Util);
    void Shutdown();
    void Render(ID3D11DeviceContext *);

    int GetIndexCount();
    XMFLOAT4 GetApexColor();
    XMFLOAT4 GetCenterColor();

private:
    void ReleaseSkyDomeModel();

    bool InitializeBuffers(ID3D11Device *);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext *);

    // Member
private:
    vector<VertexType> m_model;

    int m_vertexCount;
    int m_indexCount;

    ID3D11Buffer *m_vertexBuffer;
    ID3D11Buffer *m_indexBuffer;

    XMFLOAT4 m_apexColor;
    XMFLOAT4 m_centerColor;
};

