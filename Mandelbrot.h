#pragma once

#include <fstream>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

#include "Texture.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Mandelbrot
{
public:
    Mandelbrot();
    Mandelbrot(const Mandelbrot &);
    ~Mandelbrot();

    bool Initialize(ID3D11Device *pDevice,
                    ID3D11DeviceContext *pContext,
                    HWND hwnd,
                    WCHAR *pVsFilename,
                    WCHAR *pPsFilename,
                    WCHAR *pCsFilename);

    void Shutdown();

    ID3D11ShaderResourceView *GetDisplacementMap();

private:

    bool InitializeShader(ID3D11Device *pDevice,
                          ID3D11DeviceContext *pContext,
                          HWND hwnd,
                          WCHAR *pVsFilename,
                          WCHAR *pPsFilename,
                          WCHAR *pCsFilename);

    // Shader error output message and log.
    void OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                  HWND hwnd,
                                  WCHAR *shaderFilename);

private:

    Texture *m_pDisplacementTex;        // RGBA32F


};

