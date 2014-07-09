#pragma once

#include <vector>
#include <fstream>

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

#include "Texture.h"
#include "Math.h"
#include "Vec2f.h"

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

// Acceleration of gravity
const float GRAVITY = 981.0f;

class Ocean
{
public:
    struct OceanParameter
    {
        int displacementMapDim;
        float timeScale;
        float waveAmplitude;
        Vec2f windDir;
        float windVelo;
        float windDependency;
        float choppyScale;
    };

public:
    Ocean();
    Ocean(const Ocean &);
    ~Ocean();

    bool Initialize(OceanParameter &params,
                    ID3D11Device *device,
                    ID3D11DeviceContext *context,
                    HWND hwnd,
                    WCHAR *vsFilename,
                    WCHAR *psFilename,
                    WCHAR *csFilename);
    void Shutdown();

    bool UpdateDisplacement(float time, ID3D11DeviceContext *context);

    ID3D11ShaderResourceView *GetDisplacementMap();
    ID3D11ShaderResourceView *GetGradientMap();

    const OceanParameter &GetParameters();

private:

    // Initialize the shaders
    bool InitializeShader(ID3D11Device *device,
                          ID3D11DeviceContext *context,
                          HWND hwnd,
                          WCHAR *vsFilename,
                          WCHAR *psFilename,
                          WCHAR *csFilename);


    // Initialize a D3D11 buffer with unordered access view and shader resource view.
    bool InitializeBuffer(ID3D11Device *pd3dDevice,
                          void *data,
                          UINT byteWidth,
                          UINT byteStride,
                          ID3D11Buffer **ppBuffer,
                          ID3D11UnorderedAccessView **ppUAV,
                          ID3D11ShaderResourceView **ppSRV);

    // Clean all buffers.
    void ShutdownBuffers();

    // Initialize the vector field.
    bool InitializeHeightMap(OceanParameter &params,
                             XMFLOAT2 *heightData,
                             float *omega);

    // Generate a float based on the Phillips spectrum
    float GeneratePhillips(Vec2f waveVec,
                           Vec2f windDir,
                           float windVelo,
                           float amplitude,
                           float dirDepend);

    // Generate a random float based on Gauss distribution
    // with mean 0 and standard deviation 1.
    float GenerateGaussRand();

    // Calculate 16 bit padding.
    int CalcPad16(int n);

    // Shader error output message and log.
    void OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                  HWND hwnd,
                                  WCHAR *shaderFilename);

private:
    // Ocean parameter setup
    OceanParameter m_params;

    // textures
    //
    Texture *m_pDisplacementTex;        // RGBA32F
    Texture *m_pGradientTex;			// RGBA16F

    ID3D11SamplerState *m_pPointSampler;

    // simulation
    //
    // Initial height field H(0) (cf. eq. 25 Tessendorf)
    ID3D11Buffer *m_pBufferFloat2H0;
    ID3D11UnorderedAccessView *m_pUavH0;
    ID3D11ShaderResourceView *m_pSrvH0;

    // Angular frequency (cf. eq. 9, 10 Tessendorf)
    ID3D11Buffer *m_pBufferFloatOmega;
    ID3D11UnorderedAccessView *m_pUavOmega;
    ID3D11ShaderResourceView *m_pSrvOmega;

    // Frequency Domain
    //
    // Height field H(t) in frequency domain
    ID3D11Buffer *m_pBufferFloat2Ht;
    ID3D11UnorderedAccessView *m_pUavHt;
    ID3D11ShaderResourceView *m_pSrvHt;
    // Choppy field Dx(t) in frequency domain.
    ID3D11Buffer *m_pBufferFloat2Dxt;
    ID3D11UnorderedAccessView *m_pUavDxt;
    ID3D11ShaderResourceView *m_pSrvDxt;
    // Choppy field Dy(t) in frequency domain
    ID3D11Buffer *m_pBufferFloat2Dyt;
    ID3D11UnorderedAccessView *m_pUavDyt;
    ID3D11ShaderResourceView *m_pSrvDyt;

    // Space domain
    //
    // Choppy buffer in the space domain, corresponding to Dx(t).
    ID3D11Buffer *m_pBufferFloat2Dx;
    ID3D11UnorderedAccessView *m_pUavDx;
    ID3D11ShaderResourceView *m_pSrvDx;
    // Choppy buffer in the space domain, corresponding to Dy(t).
    ID3D11Buffer *m_pBufferFloat2Dy;
    ID3D11UnorderedAccessView *m_pUavDy;
    ID3D11ShaderResourceView *m_pSrvDy;
    // Height buffer in the space domain, corresponding to H(t).
    ID3D11Buffer *m_pBufferFloat2Dz;
    ID3D11UnorderedAccessView *m_pUavDz;
    ID3D11ShaderResourceView *m_pSrvDz;

    ID3D11Buffer *m_pQuadVB;

    // Shaders, layouts and constants
    ID3D11ComputeShader *m_pSimulationCS;

    ID3D11VertexShader *m_pQuadVS;
    ID3D11PixelShader *m_pUpdateDisplacementPS;
    ID3D11PixelShader *m_pNormalsFoldingsPS;

    ID3D11InputLayout *m_pVSLayout;

    // constant buffers
    ID3D11Buffer *m_pImmutableConstBuf;
    ID3D11Buffer *m_pPerFrameConstBuf;

    // FFT
    //
    // CSFFT512x512_Plan m_fftPlan;
};

