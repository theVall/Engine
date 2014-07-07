#include "Ocean.h"


Ocean::Ocean()
{
}


Ocean::Ocean(const Ocean &)
{
}


Ocean::~Ocean()
{
}

bool Ocean::InitializeBuffer(ID3D11Device* device,
                             void* data,
                             UINT byteWidth,
                             UINT byteStride,
                             ID3D11Buffer** ppBuffer,
                             ID3D11UnorderedAccessView** ppUAV,
                             ID3D11ShaderResourceView** ppSRV)
{
    HRESULT result;

    // Create buffer
    D3D11_BUFFER_DESC bufDesc;
    bufDesc.ByteWidth = byteWidth;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = byteStride;

    D3D11_SUBRESOURCE_DATA initData = { data, 0, 0 };

    result = device->CreateBuffer(&bufDesc, data != NULL ? &initData : NULL, ppBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Create unordered access view
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = byteWidth / byteStride;
    uavDesc.Buffer.Flags = 0;

    result = device->CreateUnorderedAccessView(*ppBuffer, &uavDesc, ppUAV);
    if (FAILED(result))
    {
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = byteWidth / byteStride;

    result = device->CreateShaderResourceView(*ppBuffer, &srvDesc, ppSRV);
    if (FAILED(result))
    {
        return false;
    }
    return true;
}


float Ocean::GenerateGaussRand()
{
    float r1 = rand() / (float)RAND_MAX;
    float r2 = rand() / (float)RAND_MAX;

    if (r1 < 1e-6f)
    {
        r1 = 1e-6f;
    }

    return sqrtf(-2.0f * logf(r1)) * cosf(2.0f * F_PI * r2);
}


float Ocean::GeneratePhillipsSpectrum(Vec2f waveVec,
                                      Vec2f windDir,
                                      float windVelo,
                                      float amplitude,
                                      float dirDepend)
{
    // largest possible wave
    float L = windVelo * windVelo / GRAVITY;
    float damping = L / 1000.0f;

    float waveSqr = waveVec.x * waveVec.x + waveVec.y * waveVec.y;
    float omega = waveVec.x * windDir.x + waveVec.y * windDir.y;

    float phillips = amplitude * expf(-1.0f / (L * L * waveSqr)) /
                     (waveSqr * waveSqr * waveSqr) * (omega * omega);

    // filter out waves moving opposite wind
    if (omega < 0)
    {
        phillips *= dirDepend;
    }

    // damp out waves with very small length
    return phillips * expf(-waveSqr * damping * damping);
}


bool Ocean::InitializeHeightMap(OceanParameter &params,
                                XMFLOAT2* heightData,
                                float* omega)
{
    Vec2f waveVec;
    Vec2f windDir;

    windDir.Normalize();

    float amplitude = params.waveAmplitude * 1e-7f;
    float windVelo = params.windVelo;
    float dirDepend = params.windDependency;

    int heightMapDim = params.displacementMapDim;

    int j;
    int index;
    float phil;

    // Seed random generator
    srand(0);

    // height map must be squared
    for (int i = 0; i < heightMapDim - 1; ++i)
    {
        waveVec.y = (-heightMapDim / 2.0f + i) * (2.0f * F_PI);

        for (j = 0; i < heightMapDim - 1; ++i)
        {
            waveVec.x = (-heightMapDim / 2.0f + j) * (2.0f * F_PI);

            if (waveVec.x == 0.0f && waveVec.y == 0.0f)
            {
                phil = 0.0f;
            }
            else
            {
                phil = sqrtf(GeneratePhillipsSpectrum(waveVec,
                                                      windDir,
                                                      windVelo,
                                                      amplitude,
                                                      dirDepend));
            }

            index = i * (heightMapDim + 4) + j;

            // initial height H0 (cf. eq. 25 Tessendorf)
            heightData[index].x = float(phil * GenerateGaussRand() * F_HALF_SQRT_2);
            heightData[index].y = float(phil * GenerateGaussRand() * F_HALF_SQRT_2);
            
            // Gerstner wave (cf. eq. 9, 10 Tessendorf)
            omega[index] = sqrtf(GRAVITY * sqrtf(waveVec.x * waveVec.x + waveVec.y * waveVec.y));
        }
    }
    return true;
}


ID3D11ShaderResourceView* Ocean::GetDisplacementMap()
{
    return m_pDisplacementMapSrv;
}


ID3D11ShaderResourceView* Ocean::GetGradientMap()
{
    return m_pGradientMapSrv;
}


const Ocean::OceanParameter& Ocean::GetParameters()
{
    return m_params;
}


int Ocean::CalcPad16(int n)
{
    return ((n + 15) / 16*16);
}