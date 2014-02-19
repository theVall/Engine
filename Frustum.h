#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

const int NUM_PLANES = 6;

class Frustum
{
public:

    Frustum();
    Frustum(const Frustum &);
    ~Frustum();

    void ConstructFrustum(XMMATRIX &projectionMatrix, XMMATRIX &viewMatrix, float screenDepth);

    bool CheckPoint(float x, float y, float z);
    bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
    bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
    bool CheckRectangle(float xCenter,
                        float yCenter,
                        float zCenter,
                        float xSize,
                        float ySize,
                        float zSize);

private:

    // Inline functions for vector calculations.
    inline XMFLOAT4 mul(float s, const XMFLOAT4 &v)
    {
        return XMFLOAT4(s*v.x, s*v.y, s*v.z, s*v.w);
    }
    inline float dot(const XMFLOAT4 &a, const XMFLOAT4 &b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    }
    inline float len(const XMFLOAT4 &a)
    {
        return sqrtf(dot(a, a));
    }
    inline XMFLOAT4 normalize(const XMFLOAT4 &a)
    {
        return mul(1.0f/len(a), a);
    }
    inline float planeDot(const XMFLOAT4 &p, const XMFLOAT3 &v)
    {
        return (p.x*v.x + p.y*v.y + p.z*v.z + p.w);
    }

private:

    XMFLOAT4 m_planes[NUM_PLANES];

};

