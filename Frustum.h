#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Plane.h"

using namespace math;
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

    bool CheckPoint(Vec3f vec);
    bool CheckCube(Vec3f center, float radius);
    bool CheckSphere(Vec3f center, float radius);
    bool CheckCuboid(Vec3f center, Vec3f size);

private:

    Plane m_planes[NUM_PLANES];

};

