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

    void SetAmbientColor(float, float, float, float);
    void SetDiffuseColor(float, float, float, float);
    void SetDirection(float, float, float);
    void SetSpecularColor(float, float, float, float);
    void SetSpecularPower(float);

    XMFLOAT4 GetAmbientColor();
    XMFLOAT4 GetDiffuseColor();
    XMFLOAT3 GetDirection();
    XMFLOAT4 GetSpecularColor();
    float GetSpecularPower();

private:
    XMFLOAT4 m_ambientColor;
    XMFLOAT4 m_diffuseColor;
    XMFLOAT4 m_specularColor;
    XMFLOAT3 m_direction;
    float m_specularPower;
};