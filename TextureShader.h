#pragma once

#include "ShaderProgram.h"

class TextureShader :
    public ShaderProgram
{
public:
    TextureShader();
    TextureShader(const TextureShader &);
    ~TextureShader();

    bool Render(ID3D11DeviceContext *,
                int,
                const XMMATRIX &,
                const XMMATRIX &,
                const XMMATRIX &,
                ID3D11ShaderResourceView *);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    bool SetShaderParameters(ID3D11DeviceContext*,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             const XMMATRIX &,
                             ID3D11ShaderResourceView *);

    void RenderShader(ID3D11DeviceContext*, int);
};


