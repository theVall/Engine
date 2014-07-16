#include "OceanShader.h"


OceanShader::OceanShader()
{
}


OceanShader::OceanShader(const OceanShader &)
{
}


OceanShader::~OceanShader()
{
}


bool OceanShader::InitializeShader(ID3D11Device *pDevice,
                                   HWND hwnd,
                                   WCHAR *vsFilename,
                                   WCHAR *psFilename)
{
    return true;
}


void OceanShader::ShutdownShader()
{

}


bool OceanShader::Render(ID3D11DeviceContext *pContext,
                         const XMMATRIX &worldMatrix,
                         const XMMATRIX &viewMatrix,
                         const XMMATRIX &projectionMatrix)
{
    return true;
}


void RenderShader(ID3D11DeviceContext *pContext, int indexCount)
{

}