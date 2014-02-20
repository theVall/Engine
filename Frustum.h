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
    bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
    bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
    bool CheckRectangle(float xCenter,
                        float yCenter,
                        float zCenter,
                        float xSize,
                        float ySize,
                        float zSize);

private:

    Plane m_planes[NUM_PLANES];

};

