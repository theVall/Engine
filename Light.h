#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class Light
{
public:
    Light();
    Light(const Light &);
    ~Light();

    void SetDiffuseColor(float, float, float, float);
    void SetDirection(float, float, float);

    XMFLOAT4 GetDiffuseColor();
    XMFLOAT3 GetDirection();

private:
    XMFLOAT4 m_diffuseColor;
    XMFLOAT3 m_direction;
};